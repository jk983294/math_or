#include <IpSolver.h>
#include <stdexcept>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <IpHelper.h>

namespace ips {
ResultStatus IpSolver::Solve() {
    if (not ReFormula()) return NOT_SOLVED;
    std::cout << "vars:\n" << to_string_or_variables() << std::endl;
    std::cout << "obj: " << to_string_or_objective() << std::endl;
    std::cout << "cons:\n" << to_string_or_constraint() << std::endl;
    m_obj_value = std::numeric_limits<int64_t>::max();

    int var_num = m_var_n - m_var_offset;
    if (var_num == 0) {
        // TODO check if no var left
    }
    choice.resize(m_var_n, 0);
    m_solution_idx.resize(m_var_n, 0);
    values.resize(m_var_n);
    uint64_t total = 1;
    m_calc_cnt = 0;
    for (int32_t i = m_var_offset; i < m_var_n - 1; i++) {
        total *= m_lens[i];
    }
    auto* pChoice = choice.data() + m_var_offset;
    auto* pMaxChoice = m_lens.data() + m_var_offset;
    auto* pValue = values.data() + m_var_offset;

    int64_t vol_lb = 0;
    { // to lower bound
        fill_value();
        calc_constrains();
        vol_lb = m_vol_diff + m_vol_target;
        if (vol_lb > m_vol_target) {
            return NOT_SOLVED;
        }
        int idx_ = var_num - 1;
        for (; idx_ >= 0; idx_--) {
            int64_t deficity = m_vol_target - vol_lb;
            int len_idx = deficity / m_v_tick_size;
            int32_t max_len = pMaxChoice[idx_];
            if (len_idx < max_len) {
                pChoice[idx_] = len_idx;
                pValue[idx_] += m_v_tick_size * len_idx;
                vol_lb += m_v_tick_size * len_idx;
                break;
            } else {
                pChoice[idx_] = max_len - 1;
                values[m_var_offset + idx_] += m_v_tick_size * (max_len - 1);
                vol_lb += m_v_tick_size * (max_len - 1) * m_cons[0].m_coefs[m_var_offset + idx_];
            }
            // printf("%ld,%ld\n", deficity, vol_lb);
        }
        if (idx_ < 0) {
            return NOT_SOLVED;
        }
    }

    int32_t max_len_least_level = pMaxChoice[var_num - 1];
    int64_t last_amt_diff = 0;
    bool good = false;
    do {
        int64_t deficity = m_vol_target - vol_lb;
        int len_idx = deficity / m_v_tick_size;
        if (len_idx <= max_len_least_level) {
            if (len_idx > 0) {
                pChoice[var_num - 1] = len_idx;
                pValue[var_num - 1] += m_v_tick_size * len_idx;
                vol_lb += m_v_tick_size * len_idx;
            }
            calc_constrains();
            if (m_vol_diff == 0) {
                // printf("%d,%d,%d,%ld\n", pChoice[var_num - 3], pChoice[var_num - 2], pChoice[var_num - 1], m_amt_diff);
                if (m_amt_diff == 0) {
                    int64_t new_obj = calc_objective();
                    if (m_obj_value > new_obj) {
                        m_obj_value = new_obj;
                        std::copy(choice.data() + m_var_offset, choice.data() + m_var_n, m_solution_idx.data() + m_var_offset);
                    }
                }
                
            }
        } else {
            printf("h\n");
        }

        m_calc_cnt++;
        if (m_calc_cnt % 10000 == 0) {
            printf("n=%zu/%zu\n", m_calc_cnt, total);
        }

        good = false;
        int check_bit = 1;
        if (last_amt_diff != 0 && m_amt_diff != 0) {
            if ((last_amt_diff > m_amt_diff && m_amt_diff < 0) || (m_amt_diff > last_amt_diff && last_amt_diff > 0)) {
                check_bit = 2;
            }
        }
        for (int i = var_num - check_bit - 1; i >= 0; --i) {
            if (pChoice[i] != pMaxChoice[i] - 1) {
                ++pChoice[i];
                pValue[i] += m_v_tick_size;
                vol_lb += m_v_tick_size;
                for (int j = i + 1; j < var_num; j++) {
                    int64_t v_delta = m_v_tick_size * pChoice[j];
                    pValue[j] -= v_delta;
                    vol_lb -= v_delta;
                    pChoice[j] = 0;
                }
                good = true;
                break;
            }
        }
        if (check_bit == 1) last_amt_diff = m_amt_diff;
        else last_amt_diff = 0;
    } while (good);
    
    if (m_obj_value != std::numeric_limits<int64_t>::max()) {
        for (int32_t i = 0; i < m_var_n; i++) {
            int64_t fix_val = m_lbs[i] + m_v_tick_size * m_solution_idx[i];
            m_solution[m_var_reorder_idx[i]] = fix_val;
        }
        return OPTIMAL;
    } else {
        return NOT_SOLVED;
    }
}

int64_t IpSolver::calc_objective() {
    int64_t result = 0;
    for (int j = m_var_offset; j < m_var_n; ++j) {
        result += m_obj_coefs[j] * values[j];
    }
    return result;
}

void IpSolver::fill_value() {
    for (int32_t i = m_var_offset; i < m_var_n; i++) {
        values[i] = m_lbs[i] + m_v_tick_size * choice[i];
    }
}

void IpSolver::calc_constrains() {
    m_vol_diff = 0;
    m_amt_diff = 0;
    for (int i = m_var_offset; i < m_var_n; ++i) {
        // m_vol_diff += m_cons[0].m_coefs[i] * values[i];
        m_vol_diff += values[i];
        m_amt_diff += m_cons[1].m_coefs[i] * values[i];
    }
    m_vol_diff -=  m_vol_target;
    m_amt_diff -= m_amt_target;
}

int64_t IpSolver::ObjValue() const {
    return m_obj_value;
}
const int64_t* IpSolver::Solution() const {
    return m_solution.data();
}

bool IpSolver::ReFormula() {
    if (m_cons.size() != 2) {
        throw std::runtime_error("expect two valid constrains");
    }
    for (size_t i = 0; i < 2; i++) {
        if (not m_cons[i].IsEquality()) {
            throw std::runtime_error("expect cons " + std::to_string(i) + " IsEquality");
        }
    }
    int half_n = m_var_n / 2;

    for (int32_t i = 0; i < m_var_n; i++) {
        auto lb_ = m_lbs[i];
        auto ub_ = m_ubs[i];
        if (lb_ % m_v_tick_size != 0 || ub_ % m_v_tick_size != 0) {
            throw std::runtime_error("expect var " + std::to_string(i) + " lb/ub by tick_size");
        }
        if (lb_ > ub_) {
            throw std::runtime_error("expect var " + std::to_string(i) + " lb <= ub");
        }
        if (i >= half_n) { // half neg
            m_lbs[i] = -ub_;
            m_ubs[i] = -lb_;
            m_cons[0].m_coefs[i] = -m_cons[0].m_coefs[i];
            m_cons[1].m_coefs[i] = -m_cons[1].m_coefs[i];
            m_obj_coefs[i] = -m_obj_coefs[i];
        }
    }

    std::vector<VarSort> m_var_sorts;
    for (int32_t i = 0; i < m_var_n; i++) {
        m_var_sorts.push_back(VarSort{i, m_lens[i]});
    }
    std::sort(m_var_sorts.begin(), m_var_sorts.end(), [](auto& l, auto& r) {
        return l.len < r.len;
    });
    for (int i = 0; i < m_var_n; i++) {
        m_var_reorder_idx[i] = m_var_sorts[i].idx;
    }
    
    std::vector<int64_t> tmp(m_var_n);
    std::vector<int32_t> tmp_int(m_var_n);
    reorder_inplace(m_obj_coefs.data(), tmp.data(), m_var_reorder_idx.data(), m_var_n);
    reorder_inplace(m_lbs.data(), tmp.data(), m_var_reorder_idx.data(), m_var_n);
    reorder_inplace(m_ubs.data(), tmp.data(), m_var_reorder_idx.data(), m_var_n);
    reorder_inplace(m_lens.data(), tmp_int.data(), m_var_reorder_idx.data(), m_var_n);
    for (size_t i = 0; i < 2; i++) {
        auto& coefs = m_cons[i].m_coefs;
        reorder_inplace(coefs.data(), tmp.data(), m_var_reorder_idx.data(), m_var_n);
    }
    for (; m_var_offset < m_var_n; m_var_offset++) {
        if (m_var_sorts[m_var_offset].len > 1) {
            break;
        }
    }
    m_vol_target = m_cons[0].m_ub;
    m_amt_target = m_cons[1].m_ub;
    m_solution.resize(m_var_n);
    for (int i = 0; i < m_var_offset; i++) {
        int64_t fix_val = m_lbs[i];
        if (fix_val != 0) {
            m_vol_target -= fix_val * m_cons[0].m_coefs[i];
            m_amt_target -= fix_val * m_cons[1].m_coefs[i];
        }
    }
    return true;
}

void IpSolver::init(int n_var, int n_constraint) {
    m_var_n = n_var;
    if (m_var_n % 2 != 0) {
        throw std::runtime_error("must even");
    }
    m_lbs.resize(n_var);
    m_ubs.resize(n_var);
    m_lens.resize(n_var);
    m_obj_coefs.resize(n_var);
    m_var_reorder_idx.resize(n_var, 0);
    std::fill(m_obj_coefs.begin(), m_obj_coefs.end(), 0);

    m_cons.resize(n_constraint);
    for (int i = 0; i < n_constraint; i++) {
        m_cons[i].Resize(m_var_n);
    }
}
void IpSolver::SetVarBound(int idx, int64_t lb, int64_t ub) {
    if (lb * ub < 0) {
        throw std::runtime_error("not one side!");
    }
    m_lbs[idx] = lb;
    m_ubs[idx] = ub;
    m_lens[idx] = (ub - lb) / m_v_tick_size + 1L;
}
void IpSolver::SetConstraintBound(int idx, int64_t lb, int64_t ub) {
    m_cons[idx].SetBound(lb, ub);
}
void IpSolver::SetConstraintCoef(int c_idx, int v_idx, int64_t coef) {
    m_cons[c_idx].m_coefs[v_idx] = coef;
}
void IpSolver::SetConstraintCoefs(int c_idx, const std::vector<int64_t>& coefs) {
    m_cons[c_idx].m_coefs = coefs;
}
void IpSolver::SetObjectiveCoefs(const std::vector<int64_t>& coefs) {
    if (coefs.size() != m_obj_coefs.size()) {
        throw std::runtime_error("SetObjectiveCoefs size incorrect!");
    }
    m_obj_coefs = coefs;
}
void IpSolver::SetVarBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) {
    if ((int)lbs.size() != m_var_n || (int)ubs.size() != m_var_n) {
        throw std::runtime_error("SetVarBounds size incorrect!");
    }
    for (int i = 0; i < m_var_n; ++i) {
        SetVarBound(i, lbs[i], ubs[i]);
    }
}
void IpSolver::SetConstraintBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) {
    if (lbs.size() != m_cons.size() || ubs.size() != m_cons.size()) {
        throw std::runtime_error("SetConstraintBounds size incorrect!");
    }
    for (uint64_t i = 0; i < lbs.size(); ++i) {
        m_cons[i].SetBound(lbs[i], ubs[i]);
    }
}
void IpSolver::SetObjectiveCoef(int idx, int coef) {
    m_obj_coefs[idx] = coef;
}
std::string IpSolver::to_string_or_variables() {
    std::string str;
    for (int i = 0; i < m_var_n; ++i) {
        std::string tmp;
        tmp += "x" + std::to_string(i) + ", lb=";
        tmp += std::to_string(m_lbs[i]);
        tmp += ", ub=";
        tmp += std::to_string(m_ubs[i]);
        tmp += ", len=" + std::to_string(m_lens[i]);
        tmp += ", idx=" + std::to_string(m_var_reorder_idx[i]);

        str += tmp;
        if (i + 1 != m_var_n) str += "\n";
    }
    return str;
}
std::string IpSolver::to_string_or_constraint() {
    std::string str;
    for (size_t i = 0; i < m_cons.size(); ++i) {
        std::string tmp;
        auto& cons = m_cons[i];
        for (int j = 0; j < m_var_n; ++j) {
            tmp += std::to_string(cons.m_coefs[j]) + " * x" + std::to_string(j);
            if (j + 1 != m_var_n) tmp += " + ";
        }
        tmp += " = ";
        tmp += std::to_string(cons.m_ub);
        str += tmp;
        if (i + 1 != m_cons.size()) str += "\n";
    }
    return str;
}
std::string IpSolver::to_string_or_objective() {
    std::string str;
    for (int i = 0; i < m_var_n; ++i) {
        str += std::to_string(m_obj_coefs[i]) + " * x" + std::to_string(i);
        if (i + 1 != m_var_n) str += " + ";
    }
    return str;
}
std::string IpSolver::to_string_or_solution() {
    std::string str;
    for (size_t i = 0; i < m_solution.size(); ++i) {
        str += std::to_string(m_solution[i]) + ",";
        if (i + 1 != m_solution.size()) str += "";
    }
    return str;
}
}
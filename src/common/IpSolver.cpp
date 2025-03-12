#include <IpSolver.h>
#include <stdexcept>
#include <numeric>
#include <iostream>
#include <algorithm>

namespace ips {
bool IpSolver::next_choice() {
    int size = max_choice_count.size();
    for (int i = size - 1; i >= 0; --i) {
        if (choice[i] != max_choice_count[i] - 1) {
            ++choice[i];
            std::fill(choice.begin() + i + 1, choice.end(), 0);
            return true;
        }
    }
    return false;
}
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
    m_diffs.resize(m_cons_cnt, 0);
    max_choice_count.resize(var_num);
    m_idx.resize(var_num);
    m_lens.resize(var_num);
    m_lbs.resize(var_num);
    choice.resize(var_num);
    std::fill(choice.begin(), choice.end(), 0);
    values.resize(var_num);
    uint64_t cnt = 0, total = 1;
    for (int32_t i = 0; i < var_num; i++) {
        int32_t idx = m_var_sorts[m_var_offset + i].idx;
        m_idx[i] = idx;
        m_lens[i] = m_vars[idx].m_len;
        max_choice_count[i] = m_lens[i];
        m_lbs[i] = m_vars[idx].m_lb;
        total *= m_lens[i];
        values[i] = m_vars[idx].m_lb;
    }

    do {
        for (int32_t i = 0; i < var_num; i++) {
            values[i] = m_lbs[i] + m_v_tick_size * choice[i];
        }
        update_best();
        cnt++;
        if (cnt % 10000 == 0) {
            printf("n=%zu/%zu\n", cnt, total);
        }
    } while (next_choice());
    if (m_obj_value != std::numeric_limits<int64_t>::max()) {
        m_fix_solution.insert(m_fix_solution.end(), m_solution.begin(), m_solution.end());
        m_solution = m_fix_solution;
        reorder(m_fix_solution.data(), m_solution.data(), m_var_sorts.data(), m_var_n);
        return OPTIMAL;
    } else {
        return NOT_SOLVED;
    }
}

void IpSolver::update_best() {
    int64_t diff = calc_constrains();
    if (diff == 0) {
        int64_t new_obj = calc_objective();
        if (m_obj_value > new_obj) {
            m_obj_value = new_obj;
            m_solution = values;
        }
    }
}

int64_t IpSolver::calc_objective() {
    int var_num = m_var_n - m_var_offset;
    int64_t result = 0;
    for (int j = 0; j < var_num; ++j) {
        result += m_obj_coefs[j + m_var_offset] * values[j];
    }
    return result;
}

int64_t IpSolver::calc_constrains() {
    int64_t result = 0;
    for (uint64_t i = 0; i < 2; ++i) {
        int64_t tmp = 0;
        const auto& coefs = m_cons[i].m_coefs;
        int var_num = m_var_n - m_var_offset;
        for (int j = 0; j < var_num; ++j) {
            tmp += coefs[j + m_var_offset] * values[j];
        }
        m_diffs[i] = tmp - m_cons[i].m_lb;
        result += std::abs(m_diffs[i]);
    }
    return result;
}

int64_t IpSolver::ObjValue() const {
    return m_obj_value;
}
const int64_t* IpSolver::Solution() const {
    return m_solution.data();
}

bool IpSolver::ReFormula() {
    size_t var_n = m_vars.size();
    m_cons_cnt = m_cons.size();
    for (auto& cons : m_cons) {
        size_t z_cnt = cons.CoefZeroCount();
        if (z_cnt + 1 == var_n) {
            int idx = cons.OneVarIndex();
            if (idx < 0) {
                throw std::runtime_error("expect one var");
            }
            int64_t coef = cons.m_coefs[idx];
            auto& _var = m_vars[idx];
            if (coef == 1) {
                int64_t o_lb = std::max(_var.m_lb, cons.m_lb);
                int64_t o_ub = std::min(_var.m_ub, cons.m_ub);
                if (o_lb > o_ub) return false;
                if (o_lb % m_v_tick_size != 0 || o_ub % m_v_tick_size != 0) {
                    throw std::runtime_error("expect var " + std::to_string(idx) + " lb/ub by tick_size");
                }
                _var.SetBound(o_lb, o_ub);
                cons.m_valid = false;
                m_cons_cnt--;
            } else if (coef == -1) {
                int64_t o_lb = std::max(_var.m_lb, -cons.m_ub);
                int64_t o_ub = std::min(_var.m_ub, -cons.m_lb);
                if (o_lb > o_ub) return false;
                if (o_lb % m_v_tick_size != 0 || o_ub % m_v_tick_size != 0) {
                    throw std::runtime_error("expect var " + std::to_string(idx) + " lb/ub by tick_size");
                }
                _var.SetBound(o_lb, o_ub);
                cons.m_valid = false;
                m_cons_cnt--;
            } else {
                throw std::runtime_error("expect coef 1/-1");
            }
        }
    }
    if (m_cons_cnt != m_cons.size()) {
        uint64_t valid_cnt = 0;
        for (size_t i = 0; i < m_cons_cnt; i++) {
            if (m_cons[i].m_valid) valid_cnt++;
            if (not m_cons[i].IsEquality()) {
                throw std::runtime_error("expect cons " + std::to_string(i) + " IsEquality");
            }
        }
        if (valid_cnt != m_cons_cnt) {
            throw std::runtime_error("expect all valid constrain in front");
        }
    }

    if (m_cons_cnt != 2) {
        throw std::runtime_error("expect two valid constrains");
    }

    for (size_t i = 0; i < m_cons_cnt; i++) {
        auto& coefs = m_cons[i].m_coefs;
        for (size_t j = 0; j < var_n; j++) {
            if (coefs[j] < 0L) {
                uint64_t neg_cnt = 0;
                for (size_t k = 0; k < m_cons_cnt; k++) {
                    if (m_cons[k].m_coefs[j] <= 0) neg_cnt++;
                }
                if (neg_cnt == m_cons_cnt) {
                    if (!m_vars[j].IsBoundOneSide()) {
                        throw std::runtime_error("expect var " + std::to_string(j) + " Bound One Side");
                    }
                    m_vars[j].SetBound(-m_vars[j].m_ub, -m_vars[j].m_lb);
                    for (size_t k = 0; k < m_cons_cnt; k++) {
                        m_cons[k].m_coefs[j] = -m_cons[k].m_coefs[j];
                    }
                    m_vars[j].isNeg = true;
                    m_obj_coefs[j] = -m_obj_coefs[j];
                } else {
                    throw std::runtime_error("expect all constrains' coef " + std::to_string(j) + " <= 0");
                }
            }
        }
    }

    for (size_t j = 0; j < var_n; j++) {
        if (not m_vars[j].IsBoundOneSide()) throw std::runtime_error("expect var " + std::to_string(j) + " Bound One Side");
        if (m_vars[j].m_lb % m_v_tick_size != 0 || m_vars[j].m_ub % m_v_tick_size != 0) {
            throw std::runtime_error("expect var " + std::to_string(j) + " lb/ub by tick_size");
        }
        if (m_vars[j].m_lb > m_vars[j].m_ub) {
            throw std::runtime_error("expect var " + std::to_string(j) + " lb <= ub");
        }
        m_vars[j].m_len = (m_vars[j].m_ub - m_vars[j].m_lb) / m_v_tick_size + 1L;
    }

    m_var_n = var_n;
    for (int32_t i = 0; i < m_var_n; i++) {
        m_var_sorts.push_back(VarSort{i, m_vars[i].m_len});
    }
    std::sort(m_var_sorts.begin(), m_var_sorts.end(), [](auto& l, auto& r) {
        return l.len < r.len;
    });
    std::vector<int64_t> tmp(m_var_n);
    reorder_inplace(m_obj_coefs.data(), tmp.data(), m_var_sorts.data(), m_var_n);
    for (size_t i = 0; i < m_cons_cnt; i++) {
        auto& coefs = m_cons[i].m_coefs;
        reorder_inplace(coefs.data(), tmp.data(), m_var_sorts.data(), m_var_n);
    }
    for (; m_var_offset < m_var_n; m_var_offset++) {
        if (m_var_sorts[m_var_offset].len > 1) {
            break;
        }
    }
    m_fix_solution.resize(m_var_offset);
    for (int i = 0; i < m_var_offset; i++) {
        for (size_t j = 0; j < m_cons_cnt; j++) {
            int64_t val_ = m_vars[m_var_sorts[i].idx].m_lb * m_cons[j].m_coefs[i];
            m_cons[j].m_lb -= val_;
            m_cons[j].m_ub -= val_;
        }
        m_fix_solution[i] = m_vars[m_var_sorts[i].idx].m_lb;
    }
    return true;
}

void IpSolver::init(int n_var, int n_constraint) {
    m_vars.resize(n_var);
    m_obj_coefs.resize(n_var);
    std::fill(m_obj_coefs.begin(), m_obj_coefs.end(), 0);

    m_cons.resize(n_constraint);
    for (int i = 0; i < n_constraint; i++) {
        m_cons[i].Resize(m_vars.size());
    }
}
void IpSolver::SetVarBound(int idx, int64_t lb, int64_t ub) {
    m_vars[idx].SetBound(lb, ub);
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
    if (lbs.size() != m_vars.size() || ubs.size() != m_vars.size()) {
        throw std::runtime_error("SetVarBounds size incorrect!");
    }
    for (uint64_t i = 0; i < lbs.size(); ++i) {
        m_vars[i].SetBound(lbs[i], ubs[i]);
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
    for (size_t j = 0; j < m_vars.size(); ++j) {
        std::string tmp;
        auto& pvar = m_vars[j];
        tmp += "x" + std::to_string(j) + ", lb=";
        tmp += std::to_string(pvar.m_lb);
        tmp += ", ub=";
        tmp += std::to_string(pvar.m_ub);
        tmp += ", neg=" + std::to_string(pvar.isNeg);
        tmp += ", len=" + std::to_string(pvar.m_len);

        str += tmp;
        if (j + 1 != m_vars.size()) str += "\n";
    }
    return str;
}
std::string IpSolver::to_string_or_constraint() {
    std::string str;
    for (size_t i = 0; i < m_cons.size(); ++i) {
        std::string tmp;
        auto& cons = m_cons[i];
        if (not cons.m_valid) continue;
        tmp += std::to_string(cons.m_lb);
        tmp += " <= ";
        for (size_t j = 0; j < m_vars.size(); ++j) {
            tmp += std::to_string(cons.m_coefs[j]) + " * x" + std::to_string(j);
            if (j + 1 != m_vars.size()) tmp += " + ";
        }
        tmp += " <= ";
        tmp += std::to_string(cons.m_ub);
        str += tmp;
        if (i + 1 != m_cons.size()) str += "\n";
    }
    return str;
}
std::string IpSolver::to_string_or_objective() {
    std::string str;
    for (size_t i = 0; i < m_vars.size(); ++i) {
        str += std::to_string(m_obj_coefs[i]) + " * x" + std::to_string(i);
        if (i + 1 != m_vars.size()) str += " + ";
    }
    return str;
}
std::string IpSolver::to_string_or_solution() {
    std::string str;
    for (size_t i = 0; i < m_solution.size(); ++i) {
        str += std::to_string(m_solution[i]) + ",";
        if (i + 1 != m_solution.size()) str += " + ";
    }
    return str;
}
}
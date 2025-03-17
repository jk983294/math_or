#include <IpSolver.h>
#include <stdexcept>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <IpHelper.h>

namespace ips {
ResultStatus IpSolver::Solve(int64_t _vol_target, int64_t _amt_target, uint64_t max_round) {
//    _vol_target = m_cons[0].m_ub;
//    _amt_target = m_cons[1].m_ub;
    if (_amt_target <= 0 || _vol_target <= 0) {
        return NOT_SOLVED;
    }

    if (m_cons.size() != 2) {
        throw std::runtime_error("expect two valid constrains");
    }
    for (size_t i = 0; i < 2; i++) {
        if (not m_cons[i].IsEquality()) {
            throw std::runtime_error("expect cons " + std::to_string(i) + " IsEquality");
        }
    }
    int half_n = m_var_n / 2;

    for (int32_t i = 0; i < half_n; i++) {
        auto lb_ = m_lbs[i];
        auto ub_ = m_ubs[i];
        auto lb1_ = m_lbs[i + half_n];
        auto ub1_ = m_ubs[i + half_n];
        if (lb_ % m_v_tick_size != 0 || ub_ % m_v_tick_size != 0 || lb1_ % m_v_tick_size != 0 || ub1_ % m_v_tick_size != 0) {
            throw std::runtime_error("expect var " + std::to_string(i) + " lb/ub by tick_size");
        }
        if (lb_ > ub_ || lb1_ > ub1_) {
            throw std::runtime_error("expect var " + std::to_string(i) + " lb <= ub");
        }
        m_lbs[i] = lb_ - ub1_;
        m_ubs[i] = ub_ - lb1_;
        m_lens[i] = ((ub_ - lb1_) - (lb_ - ub1_)) / m_v_tick_size + 1L;
    }

    for (int i = 0; i < half_n; i++) {
        int64_t fix_val = m_lbs[i];
        if (fix_val != 0) {
            _vol_target -= fix_val;
            _amt_target -= fix_val * m_cons[1].m_coefs[i];
        }
    }

//    std::cout << "vars:\n" << to_string_or_variables() << std::endl;
//    std::cout << "obj: " << to_string_or_objective() << std::endl;
//    std::cout << "cons:\n" << to_string_or_constraint() << std::endl;
    m_obj_value = std::numeric_limits<int64_t>::max();

    int var_num = half_n;
    if (var_num == 0) {
        if (_amt_target == 0 && _vol_target == 0) {
            fill_solution();
            return OPTIMAL;
        } else {
            return NOT_SOLVED;
        }
    }
    if (_amt_target < 0 || _vol_target < 0) {
        throw std::runtime_error("should not happen!");
    }
    
    choice.resize(m_var_n, 0);
    m_solution_idx.resize(m_var_n, 0);
    uint64_t total = 1;
    m_calc_cnt = 0;
    for (int32_t i = 0; i < var_num - 1; i++) {
        total *= m_lens[i];
    }
    auto* pChoice = choice.data();
    auto* pMaxChoice = m_lens.data();
    auto* pAmtCoef = m_cons[1].m_coefs.data();

    int64_t m_vol_diff = 0;
    int64_t m_amt_diff = 0;
    for (int i = 0; i < half_n; ++i) {
        int64_t vol_value_ = m_v_tick_size * choice[i];
        m_vol_diff += vol_value_;
        m_amt_diff += pAmtCoef[i] * vol_value_;
    }
    m_vol_diff -= _vol_target;
    m_amt_diff -= _amt_target;

    int64_t vol_lb = m_vol_diff + _vol_target;
    int64_t amt_lb = m_amt_diff + _amt_target;
    if (vol_lb > _vol_target) {
        return NOT_SOLVED;
    }

    int64_t min_fest_gap = m_p_tick_size * m_v_tick_size;
    if (var_num > 1) min_fest_gap = (pAmtCoef[var_num - 1] - pAmtCoef[0]) * m_v_tick_size;
    int64_t max_fest_gap = min_fest_gap;
    int32_t max_len_least_level = pMaxChoice[var_num - 1];
    int64_t last_amt_diff = 0;
    uint64_t cnt_skip = 0;
    uint64_t match_idx = 0, neg_incre_idx = 0;
    do {
        int check_bit = 1;
        int64_t deficity = _vol_target - vol_lb;
        int len_idx = static_cast<int32_t>(deficity / m_v_tick_size);
        if (len_idx >= 0) { // last price has capacity
            if (len_idx <= max_len_least_level) {
                if (len_idx > 0) {
                    pChoice[var_num - 1] = len_idx;
                    vol_lb += m_v_tick_size * len_idx;
                    amt_lb += m_v_tick_size * len_idx * static_cast<int64_t>(pAmtCoef[var_num - 1]);
                }
                m_vol_diff =  vol_lb - _vol_target;
                m_amt_diff = amt_lb - _amt_target;
                if (m_vol_diff == 0) {
                    if (m_amt_diff > 0) { // go on shift operation
                        int least_idx = var_num - 1;
                        int prev_idx = least_idx - 1;
                        while (prev_idx >= 0) {
                            if (prev_idx >= least_idx) {
                                prev_idx = least_idx - 1;
                                continue;
                            }
                            int64_t tick_diff = static_cast<int64_t>(pAmtCoef[least_idx] - pAmtCoef[prev_idx]) * m_v_tick_size;
                            int max_tick_change = static_cast<int>(m_amt_diff / tick_diff);
                            int prev_left = pMaxChoice[prev_idx] - pChoice[prev_idx] - 1;
                            int least_left = pChoice[least_idx];
                            if (max_tick_change > prev_left) {
                                max_tick_change = prev_left;
                            }
                            if (max_tick_change > least_left) {
                                max_tick_change = pChoice[least_idx];
                            }
                            if (max_tick_change > 0) {
                                pChoice[least_idx] -= max_tick_change;
                                pChoice[prev_idx] += max_tick_change;
                                amt_lb -= tick_diff * max_tick_change;
                                m_amt_diff = amt_lb - _amt_target;
                            }
                            if (m_amt_diff < tick_diff) {
                                break;
                            }
                            if (pChoice[least_idx] == 0) {
                                least_idx--;
                            } else {
                                return NOT_SOLVED; // last price's volume cannot cover all vol
                            }
                        }
                        check_bit = 2; // after shift, now safe to modify last two bits
                        if (prev_idx < 0) {
                            break;
                        }
                        // printf("adj choice=%d,%d,%d,%d,%d,%ld\n", pChoice[var_num - 5], pChoice[var_num - 4], pChoice[var_num - 3], pChoice[var_num - 2], pChoice[var_num - 1], m_amt_diff);
                        // if (pChoice[var_num - 3] == 112 && pChoice[var_num - 2] == 1 && pChoice[var_num - 1] == 90) {
                        //     printf("h\n");
                        // }
                    }

                    if (m_amt_diff == 0) {
                        int64_t new_obj = 0;
                        for (int j = 0; j < half_n; ++j) {
                            int64_t v = m_lbs[j] + m_v_tick_size * choice[j];
                            new_obj += m_obj_coefs[j] * std::abs(v);
                        }
                        if (m_obj_value > new_obj) {
                            m_obj_value = new_obj;
                            std::copy(choice.data(), choice.data() + half_n, m_solution_idx.data());
                        }
                        match_idx = m_calc_cnt;
                        min_fest_gap = 0;
                    } else if (m_amt_diff > 0) {
                        if (m_amt_diff < min_fest_gap) {
                            min_fest_gap = m_amt_diff;
                            std::copy(choice.data(), choice.data() + half_n, m_solution_idx.data());
                        }
                    }
                }
            } else {
                cnt_skip++;
            }
        } else { // last price has no capacity, increase the most left bit
            for (int idx_ = 0; idx_ < var_num; idx_++) {
                if (pChoice[idx_] > 0) { // find max col idx
                    int tmp_bits = (var_num - idx_ - 1);
                    if (check_bit < tmp_bits) check_bit = tmp_bits;
                    break;
                }
            }
        }

        bool good = false;
        if (last_amt_diff != 0 && m_amt_diff != 0) {
            if ((last_amt_diff > m_amt_diff && m_amt_diff < 0) || (m_amt_diff > last_amt_diff && last_amt_diff > 0)) {
                if (check_bit < 2) check_bit = 2;
            }
        }
        // printf("choice=%d,%d,%d,%d,%d,%ld\n", pChoice[var_num - 5], pChoice[var_num - 4], pChoice[var_num - 3], pChoice[var_num - 2], pChoice[var_num - 1], m_amt_diff);
        // if (pChoice[var_num - 5] == 0 && pChoice[var_num - 4] == 0 && pChoice[var_num - 3] == 114 && pChoice[var_num - 2] == 0 && pChoice[var_num - 1] == 89) {
        //     printf("h\n");
        // }
        
        if (last_amt_diff > m_amt_diff && m_amt_diff < 0 && last_amt_diff < 0 && m_calc_cnt != match_idx + 2UL) {
            int shift_bit = 1;
            if (neg_incre_idx + 1 == m_calc_cnt) {
                shift_bit = 0;
            }
            // printf("neg choice=%d,%d,%d,%d,%d,%ld,%zu,%zu,%zu\n", 
            //     pChoice[var_num - 5], pChoice[var_num - 4], pChoice[var_num - 3], pChoice[var_num - 2], pChoice[var_num - 1], 
            //     m_amt_diff, match_idx, neg_incre_idx, m_calc_cnt);
            for (int idx_ = 0; idx_ < var_num; idx_++) {
                if (pChoice[idx_] > 0) { // find max col idx
                    int tmp_bits = (var_num - idx_ - shift_bit);
                    if (check_bit < tmp_bits) check_bit = tmp_bits;
                    break;
                }
            }
            neg_incre_idx = m_calc_cnt;
        } else {
            neg_incre_idx = 0;
        }

        for (int i = var_num - check_bit - 1; i >= 0; --i) {
            if (pChoice[i] != pMaxChoice[i] - 1) {
                ++pChoice[i];
                vol_lb += m_v_tick_size;
                amt_lb += m_v_tick_size * static_cast<int64_t>(pAmtCoef[i]);
                for (int j = i + 1; j < var_num; j++) {
                    int64_t v_delta = m_v_tick_size * pChoice[j];
                    vol_lb -= v_delta;
                    amt_lb -= v_delta * pAmtCoef[j];
                    pChoice[j] = 0;
                }
                good = true;
                break;
            }
        }
        last_amt_diff = m_amt_diff;
        m_calc_cnt++;
        // if (m_calc_cnt % 10000 == 0) {
        //     printf("n=%zu/%zu\n", m_calc_cnt, total);
        // }
        if (!good) break;
        if (max_round > 0 && m_calc_cnt >= max_round) break;
    } while (true);
    // printf("cnt=%zu,%zu\n", cnt_skip, m_calc_cnt);
    
    if (min_fest_gap < max_fest_gap) {
        fill_solution();
        return min_fest_gap == 0 ? OPTIMAL : FEASIBLE;
    } else {
        return NOT_SOLVED;
    }
}

void IpSolver::fill_solution() {
    int32_t half_n = m_var_n / 2;
    for (int32_t i = 0; i < half_n; i++) {
        int64_t fix_val = m_lbs[i] + m_v_tick_size * m_solution_idx[i];
        m_solution[i] = fix_val;
    }
    for (int32_t i = 0; i < half_n; i++) {
        if (m_solution[i] < 0) { // give value to neg part to minimize obj
            m_solution[i + half_n] = -m_solution[i];
            m_solution[i] = 0;
        }
    }
}

int64_t IpSolver::ObjValue() const {
    return m_obj_value;
}
const int64_t* IpSolver::Solution() const {
    return m_solution.data();
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
    m_solution.resize(m_var_n, 0);
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
void IpSolver::SetConstraintCoef(int c_idx, int v_idx, int32_t coef) {
    m_cons[c_idx].m_coefs[v_idx] = coef;
}
void IpSolver::SetConstraintCoefs(int c_idx, const std::vector<int32_t>& coefs) {
    m_cons[c_idx].m_coefs = coefs;
}
void IpSolver::SetObjectiveCoefs(const std::vector<int32_t>& coefs) {
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
        str += std::to_string(m_solution[i]);
        if (i + 1 != m_solution.size()) str += ",";
    }
    return str;
}
}
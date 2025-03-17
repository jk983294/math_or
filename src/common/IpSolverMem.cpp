#include <IpSolverMem.h>
#include <stdexcept>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <IpHelper.h>

namespace ips {
ResultStatus IpSolverMem::Solve(int64_t _vol_target, int64_t _amt_target, uint64_t max_round) {
    for (int i = 0; i < m_var_n; i++) {
        int64_t fix_val = m_lbs[i];
        if (fix_val != 0) {
            _vol_target -= fix_val;
            _amt_target -= fix_val * pAmtCoef[i];
        }
    }

    m_obj_value = std::numeric_limits<int64_t>::max();

    int var_num = m_var_n;
    if (_amt_target < 0 || _vol_target < 0) {
        throw std::runtime_error("should not happen!");
    }
    
    uint64_t total = 1;
    m_calc_cnt = 0;
    for (int32_t i = 0; i < var_num - 1; i++) {
        total *= pMaxChoice[i];
    }

    int64_t m_vol_diff = 0;
    int64_t m_amt_diff = 0;
    for (int i = 0; i < var_num; ++i) {
        int64_t vol_value_ = m_v_tick_size * pChoice[i];
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
        if (len_idx >= 0) {
            if (len_idx <= max_len_least_level) {
                if (len_idx > 0) {
                    pChoice[var_num - 1] = len_idx;
                    vol_lb += m_v_tick_size * len_idx;
                    amt_lb += m_v_tick_size * len_idx * static_cast<int64_t>(pAmtCoef[var_num - 1]);
                }
                m_vol_diff =  vol_lb - _vol_target;
                m_amt_diff = amt_lb - _amt_target;
                if (m_vol_diff == 0) {
                    if (m_amt_diff > 0) {
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
                                return NOT_SOLVED;
                            }
                        }
                        check_bit = 2;
                        if (prev_idx < 0) {
                            break;
                        }
                    }

                    if (m_amt_diff == 0) {
                        int64_t new_obj = 0;
                        for (int j = 0; j < var_num; ++j) {
                            int64_t v = m_lbs[j] + m_v_tick_size * pChoice[j];
                            new_obj += m_obj_coefs[j] * std::abs(v);
                        }
                        if (m_obj_value > new_obj) {
                            m_obj_value = new_obj;
                            std::copy(pChoice, pChoice + var_num, m_solution_idx);
                        }
                        match_idx = m_calc_cnt;
                        min_fest_gap = 0;
                    } else if (m_amt_diff > 0) {
                        if (m_amt_diff < min_fest_gap) {
                            min_fest_gap = m_amt_diff;
                            std::copy(pChoice, pChoice + var_num, m_solution_idx);
                        }
                    }
                }
            } else {
                cnt_skip++;
            }
        } else {
            for (int idx_ = 0; idx_ < var_num; idx_++) {
                if (pChoice[idx_] > 0) {
                    for (int j = idx_ + 1; j < var_num; j++) {
                        int64_t v_delta = m_v_tick_size * (pMaxChoice[j] - pChoice[j] - 1);
                        vol_lb += v_delta;
                        amt_lb += v_delta * pAmtCoef[j];
                        pChoice[j] = pMaxChoice[j] - 1;
                    }
                    break;
                }
            }
        }

        bool good = false;
        if (last_amt_diff != 0 && m_amt_diff != 0) {
            if ((last_amt_diff > m_amt_diff && m_amt_diff < 0) || (m_amt_diff > last_amt_diff && last_amt_diff > 0)) {
                check_bit = 2;
            }
        }
        
        if (last_amt_diff > m_amt_diff && m_amt_diff < 0 && last_amt_diff < 0 && m_calc_cnt != match_idx + 2UL) {
            int shift_bit = 1;
            if (neg_incre_idx + 1 == m_calc_cnt) {
                shift_bit = 0;
            }
            for (int idx_ = 0; idx_ < var_num; idx_++) {
                if (pChoice[idx_] > 0) {
                    for (int j = idx_ + shift_bit; j < var_num; j++) {
                        int64_t v_delta = m_v_tick_size * (pMaxChoice[j] - pChoice[j] - 1);
                        vol_lb += v_delta;
                        amt_lb += v_delta * pAmtCoef[j];
                        pChoice[j] = pMaxChoice[j] - 1;
                    }
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
        if (!good) break;
        if (max_round > 0 && m_calc_cnt >= max_round) break;
    } while (true);
    
    if (min_fest_gap < max_fest_gap) {
        fill_solution();
        return min_fest_gap == 0 ? OPTIMAL : FEASIBLE;
    } else {
        return NOT_SOLVED;
    }
}

void IpSolverMem::fill_solution() {
    for (int32_t i = 0; i < m_var_n; i++) {
        int64_t fix_val = m_lbs[i] + m_v_tick_size * m_solution_idx[i];
        m_solution[i] = fix_val;
    }
}

int64_t IpSolverMem::ObjValue() const {
    return m_obj_value;
}
const int64_t* IpSolverMem::Solution() const {
    return m_solution;
}

void IpSolverMem::init(int n_var, int n_constraint) {
    m_var_n = n_var;
    int data_n = n_var * 3; // int64_t m_ubs + m_solution + m_lbs
    // int32_t amt_coef + obj_coef + pMaxChoice + pChoice + m_solution_idx
    if (n_var % 2 == 0) data_n += n_var / 2 * 5;
    else data_n += (n_var + 1) / 2 * 5;
    m_datum.resize(data_n);
    std::fill(m_datum.begin(), m_datum.end(), 0);
    m_ubs = m_datum.data();
    m_solution = m_ubs + n_var;
    m_lbs = m_solution + n_var;
    pAmtCoef = reinterpret_cast<int32_t*>(m_lbs + n_var);
    m_obj_coefs = pAmtCoef + n_var;
    pMaxChoice = m_obj_coefs + n_var;
    pChoice = pMaxChoice + n_var;
    m_solution_idx = pChoice + n_var;
}
void IpSolverMem::SetVarBound(int idx, int64_t lb, int64_t ub) {
    m_lbs[idx] = lb;
    m_ubs[idx] = ub;
    pMaxChoice[idx] = (ub - lb) / m_v_tick_size + 1L;
}
void IpSolverMem::SetConstraintBound(int idx, int64_t lb, int64_t ub) {
    throw std::runtime_error("not support!");
}
void IpSolverMem::SetConstraintCoef(int c_idx, int v_idx, int32_t coef) {
    pAmtCoef[v_idx] = coef;
}
void IpSolverMem::SetConstraintCoefs(int c_idx, const std::vector<int32_t>& coefs) {
    std::copy(coefs.begin(), coefs.begin() + m_var_n, pAmtCoef);
}
void IpSolverMem::SetObjectiveCoefs(const std::vector<int32_t>& coefs) {
    std::copy(coefs.begin(), coefs.begin() + m_var_n, m_obj_coefs);
}
void IpSolverMem::SetVarBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) {
    for (int i = 0; i < m_var_n; ++i) {
        SetVarBound(i, lbs[i], ubs[i]);
    }
}
void IpSolverMem::SetConstraintBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) {
    throw std::runtime_error("not support!");
}
void IpSolverMem::SetObjectiveCoef(int idx, int coef) {
    m_obj_coefs[idx] = coef;
}
std::string IpSolverMem::to_string_or_variables() {
    std::string str;
    for (int i = 0; i < m_var_n; ++i) {
        std::string tmp;
        tmp += "x" + std::to_string(i) + ", lb=";
        tmp += std::to_string(m_lbs[i]);
        tmp += ", ub=";
        tmp += std::to_string(m_ubs[i]);
        tmp += ", len=" + std::to_string(pMaxChoice[i]);

        str += tmp;
        if (i + 1 != m_var_n) str += "\n";
    }
    return str;
}
std::string IpSolverMem::to_string_or_constraint() {
    std::string str;
    for (int i = 0; i < m_var_n; ++i) {
        str += std::to_string(pAmtCoef[i]) + " * x" + std::to_string(i);
        if (i + 1 != m_var_n) str += " + ";
    }
    return str;
}
std::string IpSolverMem::to_string_or_objective() {
    std::string str;
    for (int i = 0; i < m_var_n; ++i) {
        str += std::to_string(m_obj_coefs[i]) + " * x" + std::to_string(i);
        if (i + 1 != m_var_n) str += " + ";
    }
    return str;
}
std::string IpSolverMem::to_string_or_solution() {
    std::string str;
    for (int32_t i = 0; i < m_var_n; ++i) {
        str += std::to_string(m_solution[i]);
        if (i + 1 != m_var_n) str += ",";
    }
    return str;
}

void IpSolverMem::SetVarBoundsWithNeg(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) {
    int half_n = lbs.size() / 2;
    for (int32_t i = 0; i < half_n; i++) {
        auto lb_ = lbs[i];
        auto ub_ = ubs[i];
        auto lb1_ = lbs[i + half_n];
        auto ub1_ = ubs[i + half_n];
        if (lb_ % m_v_tick_size != 0 || ub_ % m_v_tick_size != 0 || lb1_ % m_v_tick_size != 0 || ub1_ % m_v_tick_size != 0) {
            throw std::runtime_error("expect var " + std::to_string(i) + " lb/ub by tick_size");
        }
        if (lb_ > ub_ || lb1_ > ub1_) {
            throw std::runtime_error("expect var " + std::to_string(i) + " lb <= ub");
        }
        m_lbs[i] = lb_ - ub1_;
        m_ubs[i] = ub_ - lb1_;
        pMaxChoice[i] = ((ub_ - lb1_) - (lb_ - ub1_)) / m_v_tick_size + 1L;
    }
}
}
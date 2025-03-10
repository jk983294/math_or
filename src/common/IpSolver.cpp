#include <IpSolver.h>
#include <stdexcept>
#include <numeric>

namespace ips {
bool IpSolver::next_choice() {
    int size = static_cast<int>(max_choice_count.size());
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
    m_obj_value = std::numeric_limits<int64_t>::max();

    size_t var_n = m_vars.size();
    max_choice_count.resize(var_n);
    choice.resize(var_n);
    std::fill(choice.begin(), choice.end(), 0);
    values.resize(var_n);
    for (size_t i = 0; i < var_n; i++) {
        max_choice_count[i] = m_vars[i].m_len;
        values[i] = m_vars[i].m_lb;
    }
    
    // do {
    //     cout << choice << endl;
    // } while (next_choice(max_choice_count, choice));
    return NOT_SOLVED;
}

int64_t IpSolver::ObjValue() const {
    return m_obj_value;
}
std::vector<int64_t> IpSolver::Solution() const {
    return m_solution;
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

    for (size_t i = 0; i < m_cons_cnt; i++) {
        auto& coefs = m_cons[i].m_coefs;
        for (size_t j = 0; j < var_n; j++) {
            if (coefs[j] < 0L) {
                uint64_t neg_cnt = 0;
                for (size_t k = 0; k < m_cons_cnt; k++) {
                    if (m_cons[k].m_coefs[j] <= 0) neg_cnt++;
                }
                if (neg_cnt == m_cons_cnt) {
                    if (m_vars[j].IsBoundOneSide()) {
                        m_vars[j].SetBound(-m_vars[j].m_ub, -m_vars[j].m_lb);
                    } else {
                        throw std::runtime_error("expect var " + std::to_string(j) + " Bound One Side");
                    }
                    for (size_t k = 0; k < m_cons_cnt; k++) {
                        m_cons[k].m_coefs[j] = -m_cons[k].m_coefs[j];
                    }
                    m_vars[j].isNeg = true;
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
    return true;
}

void IpSolver::MakeIntVar(int n) {
    m_vars.resize(n);
    m_obj_coefs.resize(n);
    std::fill(m_obj_coefs.begin(), m_obj_coefs.end(), 0);
}
void IpSolver::SetVarBound(int idx, int64_t lb, int64_t ub) {
    m_vars[idx].SetBound(lb, ub);
}
void IpSolver::MakeRowConstraint(int n) {
    m_cons.resize(n);
    for (int i = 0; i < n; i++) {
        m_cons[i].Resize(m_vars.size());
    }
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
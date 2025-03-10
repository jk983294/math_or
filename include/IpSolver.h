#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <IpStructs.h>

namespace ips {
struct IpSolver {
    void MakeIntVar(int n);
    void SetVarBound(int idx, int64_t lb, int64_t ub);
    void MakeRowConstraint(int n);
    void SetConstraintBound(int idx, int64_t lb, int64_t ub);
    void SetConstraintCoef(int c_idx, int v_idx, int64_t coef);
    void SetConstraintCoefs(int c_idx, const std::vector<int64_t>& coefs);
    void SetObjectiveCoef(int idx, int coef);
    ResultStatus Solve();
    int64_t ObjValue() const;
    std::vector<int64_t> Solution() const;

    std::string to_string_or_variables();
    std::string to_string_or_constraint();
    std::string to_string_or_objective();
    std::string to_string_or_solution();

private:
    bool ReFormula();
    bool next_choice();

public:
    int64_t m_v_tick_size{100};
    int64_t m_p_tick_size{100};
    int64_t m_obj_value{0};
    uint64_t m_cons_cnt{0};
    std::vector<IpVar> m_vars;
    std::vector<IpConstraint> m_cons;
    std::vector<int64_t> m_obj_coefs;
    std::vector<int64_t> m_solution;
    std::vector<int64_t> values;
    std::vector<int> max_choice_count;
    std::vector<int> choice;
};
}
#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <IpStructs.h>
#include <IpSolverApi.h>

namespace ips {
struct IpSolver final : public IpSolverApi {
    void init(int n_var, int n_constraint) final;
    void SetVarBound(int idx, int64_t lb, int64_t ub) final;
    void SetConstraintBound(int idx, int64_t lb, int64_t ub) final;
    void SetConstraintCoef(int c_idx, int v_idx, int64_t coef) final;
    void SetObjectiveCoefs(const std::vector<int64_t>& coefs) final;
    void SetVarBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) final;
    void SetConstraintBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) final;
    void SetConstraintCoefs(int c_idx, const std::vector<int64_t>& coefs) final;
    void SetObjectiveCoef(int idx, int coef) final;
    ResultStatus Solve() final;
    int64_t ObjValue() const final;
    const int64_t* Solution() const final;

    std::string to_string_or_variables() final;
    std::string to_string_or_constraint() final;
    std::string to_string_or_objective() final;
    std::string to_string_or_solution() final;

private:
    bool ReFormula();
    void calc_constrains();
    void fill_value();
    void fill_solution();
    int64_t calc_objective();

public:
    int m_var_n{0};
    int m_real_n{0};
    
    int64_t m_vol_target{0};
    int64_t m_amt_target{0};
    int64_t m_vol_diff{0};
    int64_t m_amt_diff{0};
    std::vector<IpConstraint> m_cons;
    std::vector<int64_t> m_obj_coefs;
    std::vector<int64_t> m_solution;
    std::vector<int64_t> values;
    std::vector<int64_t> m_lbs; // var's lb
    std::vector<int64_t> m_ubs; // var's ub
    std::vector<int> m_var_reorder_idx;
    std::vector<int> m_lens;
    std::vector<int> choice;
    std::vector<int> m_solution_idx;
};
}
#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <string>
#include <IpStructs.h>
#include <IpSolverApi.h>

namespace ips {
struct IpSolverMem final : public IpSolverApi {
    void init(int n_var, int n_constraint) final;
    void SetVarBound(int idx, int64_t lb, int64_t ub) final;
    void SetConstraintBound(int idx, int64_t lb, int64_t ub) final;
    void SetConstraintCoef(int c_idx, int v_idx, int32_t coef) final;
    void SetObjectiveCoefs(const std::vector<int32_t>& coefs) final;
    void SetVarBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) final;
    void SetConstraintBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) final;
    void SetConstraintCoefs(int c_idx, const std::vector<int32_t>& coefs) final;
    void SetObjectiveCoef(int idx, int coef) final;
    ResultStatus Solve(int64_t _vol_target, int64_t _amt_target, uint64_t max_round) final;
    int64_t ObjValue() const final;
    const int64_t* Solution() const final;

    std::string to_string_or_variables() final;
    std::string to_string_or_constraint() final;
    std::string to_string_or_objective() final;
    std::string to_string_or_solution() final;

    void SetVarBoundsWithNeg(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs);

private:
    void fill_solution();

public:
    int m_var_n{0};
    std::vector<int64_t> m_datum;
    int64_t* m_ubs{nullptr};
    int64_t* m_solution{nullptr};
    int64_t* m_lbs{nullptr};
    int32_t* pAmtCoef{nullptr};
    int32_t* m_obj_coefs{nullptr};
    int32_t* pMaxChoice{nullptr};
    int32_t* pChoice{nullptr};
    int32_t* m_solution_idx{nullptr};
};
}
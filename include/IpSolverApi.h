#pragma once

#include <cmath>
#include <cstdint>
#include <vector>
#include <string>

namespace ips {
enum ResultStatus {
    /// optimal.
    OPTIMAL,
    /// feasible, or stopped by limit.
    FEASIBLE,
    /// the model is invalid
    INFEASIBLE,
    /// not been solved yet.
    NOT_SOLVED
};

struct IpSolverApi {
    virtual void init(int n_var, int n_constraint) = 0;
    virtual void SetVarBound(int idx, int64_t lb, int64_t ub) = 0;
    virtual void SetConstraintBound(int idx, int64_t lb, int64_t ub) = 0;
    virtual void SetConstraintCoef(int c_idx, int v_idx, int64_t coef) = 0;
    virtual void SetObjectiveCoef(int idx, int coef) = 0;
    virtual void SetObjectiveCoefs(const std::vector<int64_t>& coefs) = 0;
    virtual void SetVarBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) = 0;
    virtual void SetConstraintCoefs(int c_idx, const std::vector<int64_t>& coefs) = 0;
    virtual void SetConstraintBounds(const std::vector<int64_t>& lbs, const std::vector<int64_t>& ubs) = 0;
    virtual ResultStatus Solve() = 0;
    virtual int64_t ObjValue() const = 0;
    virtual const int64_t* Solution() const = 0;

    virtual std::string to_string_or_variables() = 0;
    virtual std::string to_string_or_constraint() = 0;
    virtual std::string to_string_or_objective() = 0;
    virtual std::string to_string_or_solution() = 0;

    int64_t m_v_tick_size{100};
    int64_t m_p_tick_size{100};
    int64_t m_obj_value{0};
};
}
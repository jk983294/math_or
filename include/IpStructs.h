#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

namespace ips {
struct VarSort {
    int32_t idx{0};
    int32_t len{0};
};

struct IpConstraint {
    int64_t m_lb{0};
    int64_t m_ub{0};
    std::vector<int32_t> m_coefs;
    void SetBound(int64_t lb, int64_t ub);
    void Resize(int n_vars);
    void Clear();
    bool IsEquality() const;
    size_t CoefZeroCount() const;
    int OneVarIndex() const;
};


}
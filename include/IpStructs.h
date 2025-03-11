#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

namespace ips {


struct IpVar {
    bool isNeg{false};
    int64_t m_lb{0};
    int64_t m_ub{0};
    int64_t m_len{0};
    void SetBound(int64_t lb, int64_t ub);
    bool IsBoundOneSide() const;
    void Clear();
};

struct IpConstraint {
    bool m_valid{true};
    int64_t m_lb{0};
    int64_t m_ub{0};
    std::vector<int64_t> m_coefs;
    void SetBound(int64_t lb, int64_t ub);
    void Resize(int n_vars);
    void Clear();
    bool IsEquality() const;
    size_t CoefZeroCount() const;
    int OneVarIndex() const;
};
}
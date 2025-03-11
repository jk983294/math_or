#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

namespace ips {
struct VarSort {
    int32_t idx{0};
    int32_t len{0};
};

struct IpVar {
    bool isNeg{false};
    int32_t m_len{0};
    int64_t m_lb{0};
    int64_t m_ub{0};
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

template <typename T>
void reorder(T* data, T* buffer, const VarSort* order, int len) {
    for (int i = 0; i < len; i++) {
        buffer[i] = data[order[i].idx];
    }
    std::copy(buffer, buffer + len, data);
}
}
#include <IpSolver.h>

namespace ips {
void IpConstraint::SetBound(int64_t lb, int64_t ub) {
    m_lb = lb;
    m_ub = ub;
}
void IpConstraint::Resize(int n_vars) {
    m_coefs.resize(n_vars);
    std::fill(m_coefs.begin(), m_coefs.end(), 0);
}
void IpConstraint::Clear() {
    m_lb = 0;
    m_ub = 0;
    m_coefs.clear();
}
bool IpConstraint::IsEquality() const {
    return m_lb == m_ub;
}
size_t IpConstraint::CoefZeroCount() const {
    size_t cnt = 0;
    for (auto v : m_coefs) {
        if (v == 0) cnt++;
    }
    return cnt;
}
int IpConstraint::OneVarIndex() const {
    for (size_t i = 0; i < m_coefs.size(); i++) {
        if (m_coefs[i] != 0) return i;
    }
    return -1;
}
}
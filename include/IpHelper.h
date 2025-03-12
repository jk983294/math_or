#pragma once

#include <cmath>
#include <cstdint>
#include <vector>

namespace ips {
template <typename T>
void reorder_inplace(T* data, T* buffer, const int* order, int len) {
    for (int i = 0; i < len; i++) {
        buffer[i] = data[order[i]];
    }
    std::copy(buffer, buffer + len, data);
}
template <typename T>
void reorder(const T* src, T* dest, const int* order, int len) {
    for (int i = 0; i < len; i++) {
        dest[order[i]] = src[i];
    }
}

inline bool next_choice(int32_t* choice, const int32_t* pMaxCount, int size) {
    for (int i = size - 1; i >= 0; --i) {
        if (choice[i] != pMaxCount[i] - 1) {
            ++choice[i];
            std::fill(choice + i + 1, choice + size, 0);
            return true;
        }
    }
    return false;
}
inline bool next_choice2(int32_t* choice, const int32_t* pMaxCount, int size) {
    for (int i = size - 2; i >= 0; --i) {
        if (choice[i] != pMaxCount[i] - 1) {
            ++choice[i];
            std::fill(choice + i + 1, choice + size, 0);
            return true;
        }
    }
    return false;
}
}
#pragma once
#include <vector>
#include <initializer_list>

namespace WebCFace {
inline namespace Common {

//! 1つの値またはvectorを持つクラス
template <typename T>
struct VectorOpt {
    T value_first;
    std::vector<T> vec;

    VectorOpt() = default;
    VectorOpt(const T &value) : value_first(value) {}
    VectorOpt(const std::initializer_list<T> &vec)
        : value_first(*vec.begin()), vec(vec) {}
    VectorOpt(const std::vector<T> &vec)
        : value_first(*vec.begin()), vec(vec) {}
    operator T() const { return value_first; }
    operator std::vector<T>() const {
        if (vec.empty()) {
            return std::vector<T>{value_first};
        } else {
            return vec;
        }
    }

    using value_type = T;
};
} // namespace Common
} // namespace WebCFace
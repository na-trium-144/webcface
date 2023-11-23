#pragma once
#include <vector>
#include <initializer_list>

namespace webcface {
inline namespace Common {

//! 1つの値またはvectorを持つクラス
template <typename T>
struct VectorOpt : public std::vector<T> {
    VectorOpt() = default;
    VectorOpt(const T &value) : std::vector<T>({value}) {}
    VectorOpt(const std::initializer_list<T> &vec) : std::vector<T>(vec) {}
    VectorOpt(const std::vector<T> &vec) : std::vector<T>(vec) {}
    operator T() const { return (*this)[0]; }
};
} // namespace Common
} // namespace webcface
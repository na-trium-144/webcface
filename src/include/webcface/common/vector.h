#pragma once
#include <vector>
#include <initializer_list>
#include <ranges>
#include <concepts>
#include "def.h"

namespace WEBCFACE_NS {
inline namespace Common {

//! 1つの値またはvectorを持つクラス
template <typename T>
struct VectorOpt : public std::vector<T> {
    VectorOpt() = default;
    /*!
     * \brief 単一の値をセットする
     *
     */
    VectorOpt(const T &value) : std::vector<T>({value}) {}
    /*!
     * \brief initializer_listで配列として値をセットする
     *
     */
    VectorOpt(const std::initializer_list<T> &vec) : std::vector<T>(vec) {}
    /*!
     * \brief 配列型の複数の値をセットする
     * \since ver1.7〜 (VectorOpt(std::vector<T>) を置き換え)
     *
     */
    template <typename R>
        requires std::ranges::range<R> &&
                 std::convertible_to<std::ranges::range_value_t<R>, T>
    VectorOpt(const R &range) : std::vector<T>() {
        this->reserve(std::ranges::size(range));
        for (const auto &v : range) {
            this->push_back(static_cast<T>(v));
        }
    }
    /*!
     * \brief 値を1つ返す
     *
     * 配列データの場合先頭の1つを返す
     * \return (*this)[0]
     *
     */
    operator T() const { return (*this)[0]; }
};
} // namespace Common
} // namespace WEBCFACE_NS

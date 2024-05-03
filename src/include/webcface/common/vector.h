#pragma once
#include <vector>
#include <initializer_list>
#include <iterator>
// #include <ranges>
#include <concepts>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
inline namespace Common {

template <typename R>
concept Range = requires(R range) {
                    begin(range);
                    end(range);
                };

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
     *
     * appleclang14でstd::ranges::rangeが使えないのでコンセプトを自前で実装している
     *
     * \since ver1.7 (VectorOpt(std::vector<T>) を置き換え)
     *
     */
    template <typename R>
    // requires std::ranges::range<R> &&
    //          std::convertible_to<std::ranges::range_value_t<R>, T>
        requires Range<R> && std::convertible_to<std::iter_value_t<R>, T>
    VectorOpt(const R &range) : std::vector<T>() {
        this->reserve(std::size(range));
        for (const auto &v : range) {
            this->push_back(static_cast<T>(v));
        }
    }
    /*!
     * \brief 生配列の値をセットする
     * \since ver1.7
     *
     */
    template <typename V, std::size_t N>
        requires std::convertible_to<V, T>
    VectorOpt(const V (&range)[N]) : std::vector<T>() {
        this->reserve(N);
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
WEBCFACE_NS_END

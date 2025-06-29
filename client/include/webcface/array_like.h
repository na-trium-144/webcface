#pragma once
#include <stdexcept>
#include <vector>
#include <array>
#include <type_traits>
#include <string>
#include "trait.h"
#include "webcface/exception.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace traits {

template <bool>
struct ArrayLikeTraitCheck {};
template <>
struct ArrayLikeTraitCheck<true> {
    using ArrayLike = TraitOkType;
};
template <bool>
struct ArraySizeTraitCheck {};
template <>
struct ArraySizeTraitCheck<true> {
    using SizeMatchOrDynamic = TraitOkType;
};

constexpr std::false_type isArrayLike(...) { return {}; }
template <typename T>
constexpr auto isArrayLike(T) -> std::bool_constant<
    std::is_convertible_v<decltype(*std::begin(std::declval<T>())), double> &&
    std::is_convertible_v<decltype(*std::end(std::declval<T>())), double>> {
    return {};
}
template <typename T>
using IsArrayLike = decltype(isArrayLike(std::declval<T>()));

constexpr std::true_type arraySizeMatch(...) { return {}; }
template <typename T, std::size_t Num>
constexpr auto arraySizeMatch(T, std::integral_constant<std::size_t, Num>)
    -> std::bool_constant<std::tuple_size<T>::value == Num> {
    return {};
}
template <typename T, std::size_t Num>
using ArraySizeMatch = decltype(arraySizeMatch(
    std::declval<T>(), std::integral_constant<std::size_t, Num>()));

/*!
 * T にstd::begin, std::endが使用可能で、要素がdoubleに変換可能なときにのみ、
 * ArrayLikeTrait<T>::ArrayLike が定義される
 *
 */
template <typename T>
struct ArrayLikeTrait : ArrayLikeTraitCheck<IsArrayLike<T>::value> {};
/*!
 * 生配列の場合std::declvalを使うとポインタになってしまうので、別で特殊化している
 *
 */
template <typename T, std::size_t N>
struct ArrayLikeTrait<T[N]>
    : ArrayLikeTraitCheck<std::is_convertible_v<T, double>> {};
template <typename T, std::size_t N>
struct ArrayLikeTrait<T (&)[N]> : ArrayLikeTrait<T[N]> {};

/*!
 * T が配列で、std::tuple_size<T>がNumと一致するか定義されないとき、
 * ArraySizeTrait<T, Num>::SizeMatchOrDynamic が定義される
 *
 */
template <typename T, std::size_t Num>
struct ArraySizeTrait : ArraySizeTraitCheck<ArraySizeMatch<T, Num>::value> {};
/*!
 * 生配列の場合std::declvalを使うとポインタになってしまうので、別で特殊化している
 *
 */
template <typename T, std::size_t N, std::size_t Num>
struct ArraySizeTrait<T[N], Num> : ArraySizeTraitCheck<N == Num> {};
template <typename T, std::size_t N, std::size_t Num>
struct ArraySizeTrait<T (&)[N], Num> : ArraySizeTrait<T[N], Num> {};

template <typename T>
std::vector<double> arrayLikeToVector(const T &array) {
    return std::vector<double>(std::begin(array), std::end(array));
}

template <std::size_t Num, typename T>
std::array<double, Num> arrayLikeToArray(const T &array) {
    std::array<double, Num> ret{};
    std::size_t n = 0;
    auto it = std::begin(array);
    for (; n < Num && it != std::end(array); ++it, ++n) {
        ret[n] = *it;
    }
    if (n == Num && it == std::end(array)) {
        return ret;
    } else {
        throw InvalidArgument(
            "array size mismatch, expected: " + std::to_string(Num) +
            ", got: " + std::to_string(n));
    }
}

} // namespace traits
WEBCFACE_NS_END

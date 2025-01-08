#pragma once
#include <stdexcept>
#include <vector>
#include <array>
#include <type_traits>
#include <iterator>
#include <string>
#include "trait.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace traits {

// 生配列はlvalueで渡さないとstd::begin()が使えない
template <
    typename T,
    std::enable_if_t<
        std::is_same_v<decltype(*std::begin(
                           std::declval<std::add_lvalue_reference_t<T>>())),
                       decltype(*std::end(
                           std::declval<std::add_lvalue_reference_t<T>>()))>,
        std::nullptr_t> = nullptr>
using ElementTypeOf =
    std::remove_const_t<std::remove_reference_t<decltype(*std::begin(
        std::declval<std::add_lvalue_reference_t<T>>()))>>;

template <typename T>
constexpr auto getSizeOf(T &&)
    -> std::integral_constant<std::size_t,
                              std::tuple_size<std::decay_t<T>>::value> {
    return {};
}
template <typename T, std::size_t N>
constexpr auto getSizeOf(T (&&)[N]) -> std::integral_constant<std::size_t, N> {
    return {};
}
template <typename T>
using SizeOf = decltype(getSizeOf(std::declval<T>()));

template <bool>
struct ArrayLikeTraitEnabler {};
template <>
struct ArrayLikeTraitEnabler<true> {
    using ArrayLike = TraitOkType;
};
template <bool Fixed, bool SizeMatch>
struct ArraySizeTraitEnabler {
    // Fixed = false
    using SizeMatchOrDynamic = TraitOkType;
};
template <>
struct ArraySizeTraitEnabler<true, false> {};
template <>
struct ArraySizeTraitEnabler<true, true> {
    using SizeMatchOrDynamic = TraitOkType;
    using SizeMatchStatic = TraitOkType;
};

constexpr std::false_type isArrayLike(...) { return {}; }
template <typename T>
constexpr auto isArrayLike(T &&)
    // 参照で受け取ることにより生配列も保持される
    -> std::bool_constant<std::is_convertible_v<ElementTypeOf<T>, double>> {
    return {};
}
template <typename T>
using IsArrayLike = decltype(isArrayLike(std::declval<T>()));
/*!
 * T にstd::begin, std::endが使用可能で、要素がdoubleに変換可能なときにのみ、
 * ArrayLikeTrait<T>::ArrayLike が定義される
 *
 * 要するに、range
 *
 */
template <typename T>
struct ArrayLikeTrait : ArrayLikeTraitEnabler<IsArrayLike<T>::value> {};

constexpr std::false_type isFixedSize(...) { return {}; }
template <typename T>
constexpr std::true_type isFixedSize(T &&, SizeOf<T> = {}) {
    return {};
}
template <typename T>
using IsFixedSize = decltype(isFixedSize(std::declval<T>()));

constexpr std::false_type arraySizeMatch(...) { return {}; }
template <typename T, std::size_t Num>
constexpr auto arraySizeMatch(T &&, std::integral_constant<std::size_t, Num>)
    -> std::bool_constant<SizeOf<T>::value == Num> {
    return {};
}
template <typename T, std::size_t Num>
using ArraySizeMatch = decltype(arraySizeMatch(
    std::declval<T>(), std::integral_constant<std::size_t, Num>()));
/*!
 * T が配列で、std::tuple_size<T>がNumと一致するか定義されないとき、
 * ArraySizeTrait<T, Num>::SizeMatchOrDynamic が定義される
 *
 */
template <typename T, std::size_t Num>
struct ArraySizeTrait : ArraySizeTraitEnabler<IsFixedSize<T>::value,
                                              ArraySizeMatch<T, Num>::value> {};


constexpr std::false_type isNestedArrayLike(...) { return {}; }
template <typename T, typename = ElementTypeOf<T>>
constexpr auto isNestedArrayLike(T &&) {
    return std::bool_constant < IsArrayLike<T>::value ||
           decltype(isNestedArrayLike(
               std::declval<ElementTypeOf<T>>()))::value > {};
}
template <typename T>
using IsNestedArrayLike = decltype(isNestedArrayLike(std::declval<T>()));
/*!
 * T にstd::begin, std::endが使用可能で、
 * 要素がNestedArrayLikeまたはArrayLikeの場合、
 * NestedArrayLikeTrait<T>::ArrayLike が定義される
 *
 */
template <typename T>
struct NestedArrayLikeTrait
    : ArrayLikeTraitEnabler<IsNestedArrayLike<T>::value> {};

constexpr std::false_type isNestedFixedSize(...) { return {}; }
template <typename T, typename = ElementTypeOf<T>>
constexpr auto isNestedFixedSize(T &&) {
    return std::bool_constant < IsFixedSize<T>::value &&
           (IsArrayLike<T>::value ||
            decltype(isNestedFixedSize(
                std::declval<ElementTypeOf<T>>))::value) > {};
}
template <typename T>
using IsNestedFixedSize = decltype(isNestedFixedSize(std::declval<T>()));

constexpr std::false_type nestedArraySizeMatch(...) { return {}; }
template <typename T, std::size_t Num>
constexpr auto nestedArraySizeMatch(T &&,
                                    std::integral_constant<std::size_t, Num>,
                                    SizeOf<T> = {}) {
    return std::bool_constant <
               (IsArrayLike<T>::value && SizeOf<T>::value == Num) ||
           (Num % SizeOf<T>::value == 0 &&
            decltype(nestedArraySizeMatch(
                std::declval<ElementTypeOf<T>>(),
                std::integral_constant<std::size_t,
                                       Num / SizeOf<T>::value>()))::value) > {};
}
template <typename T, std::size_t Num>
using NestedArraySizeMatch = decltype(nestedArraySizeMatch(
    std::declval<T>(), std::integral_constant<std::size_t, Num>()));
/*!
 * T が配列で、std::tuple_size<T>がNumと一致するか定義されないとき、
 * ArraySizeTrait<T, Num>::SizeMatchOrDynamic が定義される
 *
 */
template <typename T, std::size_t Num>
struct NestedArraySizeTrait
    : ArraySizeTraitEnabler<IsNestedFixedSize<T>::value,
                            NestedArraySizeMatch<T, Num>::value> {};


template <typename T>
void arrayLikeToVector(std::vector<double> &target, const T &array) {
    std::copy(std::begin(array), std::end(array), std::back_inserter(target));
}
template <typename T>
std::vector<double> arrayLikeToVector(const T &array) {
    return std::vector<double>(std::begin(array), std::end(array));
}
template <typename T>
void nestedArrayLikeToVector(std::vector<double> &target, const T &array) {
    if constexpr (IsArrayLike<T>::value) {
        arrayLikeToVector(target, array);
    } else {
        for (const auto &row : array) {
            nestedArrayLikeToVector(target, row);
        }
    }
}
template <typename T>
std::vector<double> nestedArrayLikeToVector(const T &array) {
    std::vector<double> vec;
    nestedArrayLikeToVector(vec, array);
    return vec;
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
        throw std::invalid_argument(
            "array size mismatch, expected: " + std::to_string(Num) +
            ", got: " + std::to_string(n));
    }
}

} // namespace traits
WEBCFACE_NS_END

#pragma once
#include "val_adaptor.h"
#include "trait.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace traits {

template <bool>
struct ConvertibleToValAdaptorVectorTraitCheck {};
template <>
struct ConvertibleToValAdaptorVectorTraitCheck<true> {
    using ConvertibleToValAdaptorVector = TraitOkType;
};

constexpr std::false_type isConvertibleToValAdaptorVector(...) { return {}; }
template <typename T>
constexpr auto isConvertibleToValAdaptorVector(T)
    -> std::bool_constant<
        std::is_convertible_v<decltype(*std::begin(std::declval<T>())),
                              ValAdaptor> &&
        std::is_convertible_v<decltype(*std::end(std::declval<T>())),
                              ValAdaptor>> {
    return {};
}
template <typename T>
using IsConvertibleToValAdaptorVector =
    decltype(isConvertibleToValAdaptorVector(std::declval<T>()));

/*!
 * T にstd::begin, std::endが使用可能で、要素がdoubleに変換可能なときにのみ、
 * ConvertibleToValAdaptorVectorTrait<T>::ConvertibleToValAdaptorVector
 * が定義される
 *
 */
template <typename T>
struct ConvertibleToValAdaptorVectorTrait
    : ConvertibleToValAdaptorVectorTraitCheck<
          IsConvertibleToValAdaptorVector<T>::value> {};
/*!
 * 生配列の場合std::declvalを使うとポインタになってしまうので、別で特殊化している
 *
 */
template <typename T, std::size_t N>
struct ConvertibleToValAdaptorVectorTrait<T[N]>
    : ConvertibleToValAdaptorVectorTraitCheck<
          std::is_convertible_v<T, double>> {};
template <typename T, std::size_t N>
struct ConvertibleToValAdaptorVectorTrait<T (&)[N]>
    : ConvertibleToValAdaptorVectorTrait<T[N]> {};

} // namespace traits

/*!
 * \brief ValAdaptorのVector
 * \since ver2.10
 *
 */
struct ValAdaptorVector {
    std::vector<ValAdaptor> vec;

    ValAdaptorVector() = default;
    ValAdaptorVector(std::initializer_list<ValAdaptor> init) : vec(init) {}
    template <typename R,
              typename traits::ConvertibleToValAdaptorVectorTrait<
                  R>::ConvertibleToValAdaptorVector = traits::TraitOk>
    ValAdaptorVector(const R &range)
        : vec(std::begin(range), std::end(range)) {}

    template <typename T>
    operator std::vector<T>() const {
        return std::vector<T>(vec.begin(), vec.end());
    }
    template <typename T, std::size_t N>
    operator std::array<T, N>() const {
        if (N != vec.size()) {
            throw std::invalid_argument(
                "array size mismatch, expected: " + std::to_string(N) +
                ", got: " + std::to_string(vec.size()));
        }
        std::array<T, N> a;
        for (std::size_t i = 0; i < N; i++) {
            a[i] = static_cast<T>(vec[i]);
        }
        return a;
    }
};

WEBCFACE_NS_END

#pragma once
#include "webcface/common/val_adaptor.h"
#include <tuple>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace traits {
template <typename T>
constexpr auto getInvokeSignature(T &&) -> decltype(&T::operator()) {
    return &T::operator();
}
template <typename Ret, typename... Args>
constexpr auto getInvokeSignature(Ret (*p)(Args...)) {
    return p;
}
template <typename T>
struct InvokeSignatureTrait {};
template <typename Ret, typename... Args>
struct InvokeSignatureTrait<Ret(Args...)> {
    using ReturnType = Ret;
    using ArgsTuple = std::tuple<std::decay_t<Args>...>;
    static constexpr std::size_t ArgsSize = std::tuple_size_v<ArgsTuple>;
    template <std::size_t Index>
    using ArgsAt = std::tuple_element_t<Index, ArgsTuple>;
};
template <typename Ret, typename T, typename... Args>
struct InvokeSignatureTrait<Ret (T::*)(Args...)>
    : InvokeSignatureTrait<Ret(Args...)> {};
template <typename Ret, typename T, typename... Args>
struct InvokeSignatureTrait<Ret (T::*)(Args...) const>
    : InvokeSignatureTrait<Ret(Args...)> {};
template <typename Ret, typename... Args>
struct InvokeSignatureTrait<Ret (*)(Args...)>
    : InvokeSignatureTrait<Ret(Args...)> {};

/*!
 * * Tは関数オブジェクト型、または関数型
 * * getInvokeSignature<T> で関数呼び出しの型 ( Ret(*)(Args...) や
 * Ret(T::*)(Args...) ) を取得し、
 * * InvokeSignatureTrait<Ret(Args...)> が各種メンバー型や定数を定義する
 *
 */
template <typename T>
using InvokeObjTrait = InvokeSignatureTrait<decltype(getInvokeSignature(
    std::declval<std::decay_t<T>>()))>;

} // namespace traits
WEBCFACE_NS_END

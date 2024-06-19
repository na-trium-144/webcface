#pragma once
#include <vector>
#include <type_traits>
#include <functional>
#include <optional>
#include <string>
#include <ostream>
#include <cstddef>
#include "val_adaptor.h"
#include <webcface/common/def.h>

#ifdef min
// clang-format off
#pragma message("warning: Disabling macro definition of 'min' and 'max', since they conflicts in webcface/func_info.h.")
// clang-format on
#undef min
#undef max
#endif

WEBCFACE_NS_BEGIN
namespace Message {
struct Arg;
}

using FuncType = std::function<ValAdaptor(const std::vector<ValAdaptor> &)>;
using FuncWrapperType =
    std::function<ValAdaptor(FuncType, const std::vector<ValAdaptor> &)>;

/*!
 * \brief 引数の情報を表す。
 *
 * func.setArg({ Arg(引数名).init(初期値).min(最小値).max(最大値), ... });
 * のように使う
 *
 */
class WEBCFACE_DLL Arg {
  protected:
    SharedString name_;
    ValType type_ = ValType::none_;
    std::optional<ValAdaptor> init_ = std::nullopt;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt;
    std::vector<ValAdaptor> option_;

  public:
    /*!
     * \brief 引数のargが持っている情報でthisを上書きする。
     *
     */
    void mergeConfig(const Arg &rhs);
    explicit Arg(ValType type) : type_(type) {}
    Arg() = default;

    Message::Arg toMessage() const;
    Arg(const Message::Arg &a);

    /*!
     * \brief 引数名を設定する。
     *
     */
    Arg(std::string_view name) : name_(name) {}
    Arg(std::wstring_view name) : name_(name) {}

    /*!
     * \brief 引数の名前を取得する。
     *
     */
    std::string name() const { return name_.decode(); }
    /*!
     * \brief 引数の名前を取得する。(wstring)
     * \since ver2.0
     */
    std::wstring nameW() const { return name_.decodeW(); }
    /*!
     * \brief 引数の型を取得する。
     *
     */
    ValType type() const { return type_; }
    /*!
     * \brief 引数の型を設定する。
     *
     */
    Arg &type(ValType type) {
        type_ = type;
        return *this;
    }
    /*!
     * \brief デフォルト値を取得する。
     *
     */
    std::optional<ValAdaptor> init() const { return init_; }
    /*!
     * \brief デフォルト値を設定する。
     *
     */
    template <typename T>
        requires std::constructible_from<ValAdaptor, T>
    Arg &init(const T &init) {
        init_ = ValAdaptor(init);
        return *this;
    }
    /*!
     * \brief 最小値を取得する。
     *
     */
    std::optional<double> min() const { return min_; }
    /*!
     * \brief 最小値を設定する。
     *
     * * string型引数の場合最小の文字数を表す。
     * * bool型引数の場合効果がない。
     * * option() はクリアされる。
     *
     */
    Arg &min(double min) {
        min_ = min;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 最大値を取得する。
     *
     */
    std::optional<double> max() const { return max_; }
    /*!
     * \brief 最大値を設定する。
     *
     * * string型引数の場合最大の文字数を表す。
     * * bool型引数の場合効果がない。
     * * option() はクリアされる。
     *
     */
    Arg &max(double max) {
        max_ = max;
        option_.clear();
        return *this;
    }
    /*!
     * \brief 引数の選択肢を取得する。
     *
     */
    const std::vector<ValAdaptor> &option() const { return option_; }
    Arg &option(const std::vector<ValAdaptor> &option) {
        option_ = option;
        min_ = max_ = std::nullopt;
        return *this;
    }
    /*!
     * \brief 引数の選択肢を設定する。
     *
     * * min(), max() はクリアされる。
     *
     */
    template <typename T>
        requires std::constructible_from<ValAdaptor, T>
    Arg &option(std::initializer_list<T> option) {
        return this->option(
            std::vector<ValAdaptor>(option.begin(), option.end()));
    }
};
WEBCFACE_DLL std::ostream &operator<<(std::ostream &os, const Arg &arg);

/*!
 * \brief 関数1つの情報を表す。関数の実体も持つ
 *
 */
struct FuncInfo {
    ValType return_type;
    std::vector<Arg> args;
    FuncType func_impl;
    FuncWrapperType func_wrapper;
    auto run(const std::vector<ValAdaptor> &args) {
        if (func_wrapper) {
            return func_wrapper(func_impl, args);
        } else {
            return func_impl(args);
        }
    }

    FuncInfo()
        : return_type(ValType::none_), args(), func_impl(), func_wrapper() {}
    FuncInfo(ValType return_type, const std::vector<Arg> &args,
             const FuncType &func_impl, const FuncWrapperType &func_wrapper)
        : return_type(return_type), args(args), func_impl(func_impl),
          func_wrapper(func_wrapper) {}

    /*!
     * \brief 任意の関数を受け取り、引数と戻り値をキャストして実行する関数を保存
     *
     */
    template <typename... Args, typename Ret>
    explicit FuncInfo(std::function<Ret(Args...)> func,
                      const FuncWrapperType &wrapper)
        : return_type(valTypeOf<Ret>()), args({Arg{valTypeOf<Args>()}...}),
          func_impl([func](const std::vector<ValAdaptor> &args_vec) {
              std::tuple<std::remove_const_t<std::remove_reference_t<Args>>...>
                  args_tuple;
              if (args_vec.size() != sizeof...(Args)) {
                  throw std::invalid_argument(
                      "requires " + std::to_string(sizeof...(Args)) +
                      " arguments, got " + std::to_string(args_vec.size()));
              }
              argToTuple(args_vec, args_tuple);
              if constexpr (std::is_void_v<Ret>) {
                  std::apply(func, args_tuple);
                  return ValAdaptor{};
              } else {
                  Ret ret = std::apply(func, args_tuple);
                  return static_cast<ValAdaptor>(ret);
              }
          }),
          func_wrapper(wrapper) {}
};

using CallerId = std::size_t;
using MemberId = unsigned int;

/*!
 * \brief 関数を呼び出すのに必要なデータ。
 *
 * client_data->client->server->clientと送られる
 *
 */
struct FuncCall {
    CallerId caller_id = 0;
    MemberId caller_member_id = 0;
    MemberId target_member_id = 0;
    SharedString field;
    std::vector<webcface::ValAdaptor> args;
};

WEBCFACE_NS_END

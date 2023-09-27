#pragma once
#include <vector>
#include <type_traits>
#include <functional>
#include <optional>
#include <string>
#include <ostream>
#include "val.h"

namespace WebCFace {
//! WebCFace::Commonはserverとclientで共通のheader-onlyなクラス
inline namespace Common {

using FuncType = std::function<ValAdaptor(const std::vector<ValAdaptor> &)>;
using FuncWrapperType =
    std::function<ValAdaptor(FuncType, const std::vector<ValAdaptor> &)>;

//! 引数の情報を表す。
//! func.setArg({ Arg(引数名).init(初期値).min(最小値).max(最大値), ... });
//! のように使う
class Arg {
  protected:
    std::string name_ = "";
    ValType type_ = ValType::none_;
    std::optional<ValAdaptor> init_ = std::nullopt;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt;
    std::vector<ValAdaptor> option_;

  public:
    //! 引数のargが持っている情報でthisを上書きする。
    void mergeConfig(const Arg &rhs) {
        if (rhs.name_ != "") {
            name_ = rhs.name_;
        }
        if (rhs.type_ != ValType::none_) {
            type_ = rhs.type_;
        }
        if (rhs.init_) {
            init(*rhs.init_);
        }
        if (rhs.min_) {
            min(*rhs.min_);
        }
        if (rhs.max_) {
            max(*rhs.max_);
        }
        if (rhs.option_.size() > 0) {
            option(rhs.option_);
        }
    }
    explicit Arg(ValType type) : type_(type) {}

    Arg() = default;
    //! 引数名を設定する。
    Arg(const std::string &name) : name_(name) {}
    Arg(const std::string &name, ValType type,
        const std::optional<ValAdaptor> &init, std::optional<double> min,
        std::optional<double> max, const std::vector<ValAdaptor> &option)
        : name_(name), type_(type), init_(init), min_(min), max_(max),
          option_(option) {}

    std::string name() const { return name_; }
    //! 引数の型を取得する。
    //! 型を手動で設定することはできない
    ValType type() const { return type_; }
    //! デフォルト値を取得する。
    std::optional<ValAdaptor> init() const { return init_; }
    //! デフォルト値を設定する。
    Arg &init(const ValAdaptor &init) {
        init_ = init;
        return *this;
    }
    //! 最小値を取得する。
    std::optional<double> min() const { return min_; }
    //! 最小値を設定する。
    //! string型引数の場合最小の文字数を表す。
    //! bool型引数の場合効果がない。
    //! option() はクリアされる。
    Arg &min(double min) {
        min_ = min;
        option_.clear();
        return *this;
    }
    //! 最大値を取得する。
    std::optional<double> max() const { return max_; }
    //! 最大値を設定する。
    //! string型引数の場合最大の文字数を表す。
    //! bool型引数の場合効果がない。
    //! option() はクリアされる。
    Arg &max(double max) {
        max_ = max;
        option_.clear();
        return *this;
    }
    //! 引数の選択肢を取得する。
    std::vector<ValAdaptor> option() const { return option_; }
    Arg &option(const std::vector<ValAdaptor> &option) {
        option_ = option;
        min_ = max_ = std::nullopt;
        return *this;
    }
    //! 引数の選択肢を設定する。
    //! min(), max() はクリアされる。
    template <typename T>
    Arg &option(std::initializer_list<T> option) {
        return this->option(
            std::vector<ValAdaptor>(option.begin(), option.end()));
    }
};
inline auto &operator<<(std::basic_ostream<char> &os, const Arg &arg) {
    os << arg.name() << "(type=" << arg.type();
    if (arg.min()) {
        os << ", min=" << *arg.min();
    }
    if (arg.max()) {
        os << ", max=" << *arg.max();
    }
    if (arg.option().size() > 0) {
        os << ", option={";
        for (std::size_t j = 0; j < arg.option().size(); j++) {
            if (j > 0) {
                os << ", ";
            }
            os << static_cast<std::string>(arg.option()[j]);
        }
        os << "}";
    }
    os << ")";
    return os;
}

//! 関数1つの情報を表す。関数の実体も持つ
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
    //! 任意の関数を受け取り、引数と戻り値をキャストして実行する関数を保存
    template <typename... Args, typename Ret>
    explicit FuncInfo(std::function<Ret(Args...)> func, FuncWrapperType wrapper)
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

//! 関数を呼び出すのに必要なデータ。client_data->client->server->clientと送られる
struct FuncCall {
    unsigned int caller_id;
    unsigned int caller_member_id;
    unsigned int target_member_id;
    std::string field;
    std::vector<WebCFace::ValAdaptor> args;
};

} // namespace Common
} // namespace WebCFace
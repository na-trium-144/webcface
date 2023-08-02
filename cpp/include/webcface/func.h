#pragma once
#include <vector>
#include <type_traits>
#include <functional>
#include <mutex>
#include <unordered_map>
#include <ostream>
#include <string>
#include <future>
#include <concepts>
#include "any_arg.h"
#include "data.h"

namespace WebCFace {
template <typename T>
AbstArgType abstTypeOf() {
    if constexpr (std::is_void_v<T>) {
        return AbstArgType::none_;
    } else if constexpr (std::is_same_v<bool, T>) {
        return AbstArgType::bool_;
    } else if constexpr (std::is_integral_v<T>) {
        return AbstArgType::int_;
    } else if constexpr (std::is_floating_point_v<T>) {
        return AbstArgType::float_;
    } else {
        return AbstArgType::string_;
    }
}
// 関数1つの情報を表す。関数の実体も持つ
struct FuncInfo {
    AbstArgType return_type;
    std::vector<AbstArgType> args_type;
    std::function<AnyArg(const std::vector<AnyArg> &)> func_impl;

    FuncInfo() : return_type(AbstArgType::none_), args_type(), func_impl() {}
    template <typename... Args, typename Ret>
    explicit FuncInfo(std::function<Ret(Args...)> func)
        : return_type(abstTypeOf<Ret>()), args_type({abstTypeOf<Args>()...}),
          func_impl([func](const std::vector<AnyArg> &args_vec) {
              std::tuple<Args...> args_tuple;
              argToTuple(args_vec, args_tuple);
              if constexpr (std::is_void_v<Ret>) {
                  std::apply(func, args_tuple);
                  return AnyArg{};
              } else {
                  Ret ret = std::apply(func, args_tuple);
                  return static_cast<AnyArg>(ret);
              }
          }) {}
};

class Func;
class FuncResult {
    int caller_id;
    std::string caller;
    std::shared_ptr<std::promise<FuncResult>> pr;

  public:
    friend Func;

    FuncResult(int caller_id, const std::string &caller,
               std::shared_ptr<std::promise<FuncResult>> pr)
        : caller_id(caller_id), caller(caller), pr(pr) {}
    AnyArg result{};
    bool found = false;
    bool is_error = false;
    std::string error_msg = "";
    std::string from, name;

    template <typename T>
        requires std::convertible_to<AnyArg, T>
    operator T() const {
        return static_cast<T>(result);
    }

    void setReady() { pr->set_value(*this); }
};
inline auto &operator<<(std::basic_ostream<char> &os, const FuncResult &data) {
    if (!data.found) {
        return os << "<func not found>";
    }
    if (data.is_error) {
        return os << "<error: " << data.error_msg << ">";
    } else {
        return os << static_cast<std::string>(data.result);
    }
}

// 実行した関数の記録、結果を保持
class FuncStore {
  public:
    using FuncType = std::function<AnyArg(std::vector<AnyArg>)>;

  private:
    std::mutex mtx;
    std::vector<FuncResult> results;

  public:
    void setFunc(const std::string &name, FuncType data);
    FuncType getFunc(const std::string &name);
    bool hasFunc(const std::string &name);
    FuncResult &addResult(const std::string &caller,
                          std::shared_ptr<std::promise<FuncResult>> pr);
    FuncResult &getResult(int caller_id);
};

// 関数1つを表すクラス
class Func : public SyncData<FuncInfo> {
    std::shared_ptr<FuncStore> func_impl_store;
    template <typename... Args, typename Ret>
    void set_impl(std::function<Ret(Args...)> func) {
        this->SyncData<DataType>::set(FuncInfo{func});
    }
    std::future<FuncResult> run_impl(const std::vector<AnyArg> &args_vec) const;
    Client *cli;

  public:
    friend Client;

    Func() {}
    Func(std::shared_ptr<SyncDataStore<DataType>> store,
         std::shared_ptr<FuncStore> func_impl_store, Client *cli,
         const std::string &from, const std::string &name)
        : SyncData<DataType>(store, from, name),
          func_impl_store(func_impl_store), cli(cli) {}

    template <typename T>
    void set(const T &func) {
        this->set_impl(std::function(func));
    }
    template <typename T>
    auto &operator=(const T &func) {
        this->set(func);
        return *this;
    }

    template <typename... Args>
    std::future<FuncResult> run(Args... args) const {
        return run_impl({AnyArg(args)...});
    }

    AbstArgType returnType() const;
    std::vector<AbstArgType> argsType() const;
};


} // namespace WebCFace
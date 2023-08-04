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
#include "val.h"
#include "data.h"

namespace WebCFace {

using FuncType = std::function<ValAdaptor(const std::vector<ValAdaptor> &)>;

//! 関数1つの情報を表す。関数の実体も持つ
struct FuncInfo {
    ValType return_type;
    std::vector<ValType> args_type;
    FuncType func_impl;

    FuncInfo() : return_type(ValType::none_), args_type(), func_impl() {}
    //! 任意の関数を受け取り、引数と戻り値をキャストして実行する関数を保存
    template <typename... Args, typename Ret>
    explicit FuncInfo(std::function<Ret(Args...)> func)
        : return_type(valTypeOf<Ret>()), args_type({valTypeOf<Args>()...}),
          func_impl([func](const std::vector<ValAdaptor> &args_vec) {
              std::tuple<Args...> args_tuple;
              argToTuple(args_vec, args_tuple);
              if constexpr (std::is_void_v<Ret>) {
                  std::apply(func, args_tuple);
                  return ValAdaptor{};
              } else {
                  Ret ret = std::apply(func, args_tuple);
                  return static_cast<ValAdaptor>(ret);
              }
          }) {}
};

class Func;

//! 関数の実行結果を表す。
/*! 実行開始時に生成、
 * 実行が完了したら結果をセットしpromiseに自身をセット、
 * futureからこれが返ってくる
 */
class FuncResult {
    int caller_id;
    std::string caller;
    std::shared_ptr<std::promise<FuncResult>> pr;
    std::string member_, name_;

  public:
    friend Func;

    FuncResult(int caller_id, const std::string &caller,
               std::shared_ptr<std::promise<FuncResult>> pr,
               const std::string &member, const std::string &name)
        : caller_id(caller_id), caller(caller), pr(pr), member_(member),
          name_(name) {}
    //! 関数の戻り値
    ValAdaptor result{};
    //! 関数が存在しなかった場合false
    bool found = false;
    //! 関数が例外を投げた場合true
    bool is_error = false;
    //! 例外の内容
    std::string error_msg = "";
    //! 関数の名前
    auto name() const { return name_; }
    //! 関数本体のあるmember
    // Member member() const { return ???; }

    //! 戻り値を任意の型で返す
    template <typename T>
        requires std::convertible_to<ValAdaptor, T>
    operator T() const {
        return static_cast<T>(result);
    }

    //! 値をセットしたら呼ぶ
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

//! FuncResultのリストを保持する。
/*! 関数の実行結果が返ってきた時参照する
 * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
 */
class FuncResultStore {
  private:
    std::mutex mtx;
    std::vector<FuncResult> results;

  public:
    //! 新しいFuncResultを生成する。
    FuncResult &addResult(const std::string &caller,
                          std::shared_ptr<std::promise<FuncResult>> pr,
                          const std::string &member, const std::string &name);
    FuncResult &getResult(int caller_id);
};

// 関数1つを表すクラス
class Func : public SyncData<FuncInfo> {

  public:
    Func() {}
    Func(Client *cli, const std::string &member, const std::string &name)
        : SyncData<DataType>(cli, member, name) {}

    //! 関数からFuncInfoを構築しセットする
    /*! Tは任意の関数
     */
    template <typename T>
    auto &set(const T &func) {
        this->SyncData<DataType>::set(FuncInfo(std::function(func)));
        return *this;
    }
    template <typename T>
    auto &operator=(const T &func) {
        return this->set(func);
    }

    template <typename... Args>
    std::future<FuncResult> run(Args... args) const {
        return run_impl({ValAdaptor(args)...});
    }
    std::future<FuncResult>
    run_impl(const std::vector<ValAdaptor> &args_vec) const;

    ValType returnType();
    std::vector<ValType> argsType();
};


} // namespace WebCFace
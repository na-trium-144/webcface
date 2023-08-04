#pragma once
#include <vector>
#include <type_traits>
#include <functional>
#include <mutex>
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include "val.h"
#include "data.h"
#include "decl.h"

namespace WebCFace {

struct FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const std::string &member,
                          const std::string &func_name)
        : std::runtime_error(
              (member == "" ? "self()" : "member(\"" + member + "\")") +
              ".func(\"" + func_name + "\") is not set") {}
};

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
          }) {}
};

/*! 非同期で実行した関数の実行結果を表す。
 *  結果はshared_futureのget()で得られる。
 *
 * リモートから呼び出しメッセージが送られてきた時非同期で実行して結果を送り返すのにもこれを利用する
 */
class AsyncFuncResult {
    //! 通し番号
    //! コンストラクタで設定する。実際はFuncResultStoreのvectorのindex
    int caller_id;
    Client *cli;
    //! 呼び出し側member 通常は自身
    std::string caller;
    //! 呼び出し対象のmemberとFunc名
    std::string member_, name_;

    std::shared_ptr<std::promise<bool>> started_;
    std::shared_ptr<std::promise<ValAdaptor>> result_;

  public:
    friend Func;
    friend Client;

    //! リモートに呼び出しメッセージが到達したときに値が入る
    //! 実行開始したらtrue, 呼び出しに失敗したらfalseが返る
    std::shared_future<bool> started;
    //! 関数の戻り値
    //! 例外が発生した場合futureの共有状態に例外が入る
    //! (→ get() で例外がthrowされる)
    std::shared_future<ValAdaptor> result;

    AsyncFuncResult(int caller_id, Client *cli, const std::string &caller,
                    const std::string &member, const std::string &name)
        : caller_id(caller_id), cli(cli), caller(caller), member_(member),
          name_(name), started_(std::make_shared<std::promise<bool>>()),
          result_(std::make_shared<std::promise<ValAdaptor>>()),
          started(started_->get_future()), result(result_->get_future()) {}

    //! 関数の名前
    auto name() const { return name_; }
    //! 関数本体のあるmember
    Member member() const;
};
auto &operator<<(std::basic_ostream<char> &os, const AsyncFuncResult &data);

//! AsyncFuncResultのリストを保持する。
/*! 関数の実行結果が返ってきた時参照する
 * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
 */
class FuncResultStore {
  private:
    Client *cli;
    std::mutex mtx;
    std::vector<AsyncFuncResult> results;

  public:
    explicit FuncResultStore(Client *cli) : cli(cli) {}

    //! 新しいAsyncFuncResultを生成する。
    AsyncFuncResult &addResult(const std::string &caller,
                               const std::string &member,
                               const std::string &name);
    AsyncFuncResult &getResult(int caller_id);
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

    //! 関数を実行する (同期)
    /*! selfの関数の場合、このスレッドで直接実行する
     * 例外が発生した場合そのままthrow, 関数が存在しない場合 FuncNotFound
     * をthrowする
     *
     * リモートの場合、関数呼び出しを送信し結果が返ってくるまで待機
     * 例外が発生した場合 runtime_error, 関数が存在しない場合 FuncNotFound
     * をthrowする
     */
    template <typename... Args>
    ValAdaptor run(Args... args) const {
        return run({ValAdaptor(args)...});
    }
    ValAdaptor run(const std::vector<ValAdaptor> &args_vec) const;
    //! run()と同じ
    template <typename... Args>
    ValAdaptor operator()(Args... args) const {
        return run(args...);
    }

    //! 関数を実行する (非同期)
    /*! 非同期で実行する
     * 戻り値やエラー、例外はAsyncFuncResultから取得する
     *
     * AsyncFuncResultはClient内部で保持されているのでClientが破棄されるまで参照は切れない
     */
    template <typename... Args>
    AsyncFuncResult &runAsync(Args... args) const {
        return runAsync({ValAdaptor(args)...});
    }
    AsyncFuncResult &runAsync(const std::vector<ValAdaptor> &args_vec) const;

    ValType returnType();
    std::vector<ValType> argsType();
};


} // namespace WebCFace
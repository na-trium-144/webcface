#pragma once
#include <vector>
#include <mutex>
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include "val.h"
#include "decl.h"

namespace WebCFace {

//! Funcの実行ができなかった場合発生する例外
//! (ValueやTextで参照先が見つからなかった場合はこれではなく単にnulloptが返る)
struct FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const std::string &member,
                          const std::string &func_name)
        : std::runtime_error(
              (member == "" ? "self()" : "member(\"" + member + "\")") +
              ".func(\"" + func_name + "\") is not set") {}
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

    AsyncFuncResult(int caller_id, Client *cli, const std::string &caller,
                    const std::string &member, const std::string &name)
        : caller_id(caller_id), cli(cli), caller(caller), member_(member),
          name_(name), started_(std::make_shared<std::promise<bool>>()),
          result_(std::make_shared<std::promise<ValAdaptor>>()),
          started(started_->get_future()), result(result_->get_future()) {}
  public:
    friend Func;
    friend Client;
    friend FuncResultStore;

    //! リモートに呼び出しメッセージが到達したときに値が入る
    //! 実行開始したらtrue, 呼び出しに失敗したらfalseが返る
    //! falseの場合resultにFuncNotFound例外が入る
    std::shared_future<bool> started;
    //! 関数の実行が完了した時戻り値が入る
    //! 例外が発生した場合例外が入る
    //! (→ get() で例外がthrowされる)
    std::shared_future<ValAdaptor> result;

    //! 関数の名前
    auto name() const { return name_; }
    //! 関数本体のあるmember
    Member member() const;
};
auto &operator<<(std::basic_ostream<char> &os, const AsyncFuncResult &data);

}

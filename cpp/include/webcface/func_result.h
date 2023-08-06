#pragma once
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include "field_base.h"
#include "common/val.h"
#include "member.h"

namespace WebCFace {

//! Funcの実行ができなかった場合発生する例外
//! (ValueやTextで参照先が見つからなかった場合はこれではなく単にnulloptが返る)
struct FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const FieldBase &base)
        : std::runtime_error((base.member_ == ""
                                  ? "self()"
                                  : "member(\"" + base.member_ + "\")") +
                             ".func(\"" + base.field_ + "\") is not set") {}
};

/*! 非同期で実行した関数の実行結果を表す。
 *  結果はshared_futureのget()で得られる。
 *
 * リモートから呼び出しメッセージが送られてきた時非同期で実行して結果を送り返すのにもこれを利用する
 */
class AsyncFuncResult : FieldBase {
    //! 通し番号
    //! コンストラクタで設定する。実際はFuncResultStoreのvectorのindex
    int caller_id;
    //! 呼び出し側member 通常は自身
    std::string caller;

    std::shared_ptr<std::promise<bool>> started_;
    std::shared_ptr<std::promise<ValAdaptor>> result_;

  public:
    //! promiseに書き込むことができるクラス
    friend class Func;
    friend class Client;

    AsyncFuncResult(int caller_id, const std::string &caller,
                    const FieldBase &base)
        : FieldBase(base), caller_id(caller_id), caller(caller),
          started_(std::make_shared<std::promise<bool>>()),
          result_(std::make_shared<std::promise<ValAdaptor>>()),
          started(started_->get_future()), result(result_->get_future()) {}

    //! リモートに呼び出しメッセージが到達したときに値が入る
    //! 実行開始したらtrue, 呼び出しに失敗したらfalseが返る
    //! falseの場合resultにFuncNotFound例外が入る
    std::shared_future<bool> started;
    //! 関数の実行が完了した時戻り値が入る
    //! 例外が発生した場合例外が入る
    //! (→ get() で例外がthrowされる)
    std::shared_future<ValAdaptor> result;

    //! 関数の名前
    std::string name() const { return field_; }
    //! 関数本体のあるmember
    Member member() const { return *this; }
};
auto &operator<<(std::basic_ostream<char> &os, const AsyncFuncResult &data);

} // namespace WebCFace

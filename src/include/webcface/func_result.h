#pragma once
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include <cstdint>
#include "field.h"
#include "common/val.h"

namespace webcface {

class Member;

/*!
 * \brief Funcの実行ができなかった場合発生する例外
 * 
 * (ValueやTextで参照先が見つからなかった場合はこれではなく単にnulloptが返る)
 * 
 */
struct FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const Common::FieldBase &base)
        : std::runtime_error("member(\"" + base.member_ + "\")" + ".func(\"" +
                             base.field_ + "\") is not set") {}
};

/*!
 * \brief 非同期で実行した関数の実行結果を表す。
 * 
 * 結果はshared_futureのget()で得られる。
 *
 * リモートから呼び出しメッセージが送られてきた時非同期で実行して結果を送り返すのにもこれを利用する
 * 
 */
class AsyncFuncResult : Field {
    /*!
     * \brief 通し番号
     * 
     * コンストラクタで設定する。
     * 実際はFuncResultStoreのvectorのindex
     * 
     */
    std::size_t caller_id;
    
    std::string caller; //< 呼び出し側member 通常は自身

    std::shared_ptr<std::promise<bool>> started_;
    std::shared_ptr<std::promise<ValAdaptor>> result_;

  public:
    friend class Func;
    friend struct Internal::ClientData;

    AsyncFuncResult(std::size_t caller_id, const std::string &caller,
                    const Field &base)
        : Field(base), caller_id(caller_id), caller(caller),
          started_(std::make_shared<std::promise<bool>>()),
          result_(std::make_shared<std::promise<ValAdaptor>>()),
          started(started_->get_future()), result(result_->get_future()) {}

    /*!
     * \brief リモートに呼び出しメッセージが到達したときに値が入る
     * 
     * \return 実行開始したらtrue, 呼び出しに失敗したらfalseが返る。
     * falseの場合resultにFuncNotFound例外が入る
     * 
     */
    std::shared_future<bool> started;
    /*!
     * \brief 関数の実行が完了した時戻り値が入る
     * 
     * 例外が発生した場合例外が入る
     * 
     */
    std::shared_future<ValAdaptor> result;

    using Field::member;
    using Field::name;
};
auto &operator<<(std::basic_ostream<char> &os, const AsyncFuncResult &data);

} // namespace webcface

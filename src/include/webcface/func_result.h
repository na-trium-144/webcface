#pragma once
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include <cstdint>
#include "field.h"
#include "common/val.h"
#include "common/def.h"

namespace WEBCFACE_NS {

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

    ValAdaptor result_val;
    wcfMultiVal c_result_val;
    std::pair<wcfStatus, wcfMultiVal *> toCVal() & {
        try {
            result_val = result.get();
            c_result_val = result_val.toCVal();
            return {WCF_OK, &c_result_val};
        } catch (const FuncNotFound &e) {
            result_val = e.what();
            c_result_val = result_val.toCVal();
            return {WCF_NOT_FOUND, &c_result_val};
        } catch (const std::exception &e) {
            result_val = e.what();
            c_result_val = result_val.toCVal();
            return {WCF_EXCEPTION, &c_result_val};
        } catch (...) {
            result_val = "unknown exception";
            c_result_val = result_val.toCVal();
            return {WCF_EXCEPTION, &c_result_val};
        }
    }

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


class FuncCallHandle {
    std::vector<ValAdaptor> args_;
    std::vector<wcfMultiVal> c_args_;
    std::shared_ptr<std::promise<ValAdaptor>> result_;

  public:
    FuncCallHandle() = default;
    FuncCallHandle(const std::vector<ValAdaptor> &args,
                   const std::shared_ptr<std::promise<ValAdaptor>> &result)
        : args_(args), c_args_(), result_(result) {}

    /*!
     * \brief 関数の引数を取得する
     *
     */
    std::vector<ValAdaptor> args() const { return args_; }
    /*!
     * \brief 関数の引数をwcfMultiValに変換して取得する
     * 引数の本体はargsが持っているので、FuncCallHandleの一時オブジェクトからは使えない
     *
     */
    std::vector<wcfMultiVal> &toCArgs() & {
        c_args_.resize(args_.size());
        for (std::size_t i = 0; i < args_.size(); i++) {
            c_args_[i] = args_[i].toCVal();
        }
        return c_args_;
    }
    /*!
     * \brief 関数の結果を送信する
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void respond(ValAdaptor value = "") {
        if (result_) {
            result_->set_value(value);
        } else {
            throw std::runtime_error("FuncCallHandle does not have valid "
                                     "pointer to function call");
        }
    }
    /*!
     * \brief 関数の結果を例外として送信する
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void reject(const std::string &message) {
        if (result_) {
            try {
                throw std::runtime_error(message);
            } catch (const std::runtime_error &) {
                result_->set_exception(std::current_exception());
            }
        } else {
            throw std::runtime_error("FuncCallHandle does not have valid "
                                     "pointer to function call");
        }
    }
};
} // namespace WEBCFACE_NS

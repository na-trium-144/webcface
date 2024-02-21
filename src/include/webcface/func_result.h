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
    std::size_t caller_id;

  public:
    friend class Func;

    AsyncFuncResult(const Field &base, std::size_t caller_id,
                    const std::shared_future<bool> &started_f,
                    const std::shared_future<ValAdaptor> &result_f)
        : Field(base), caller_id(caller_id), started(started_f),
          result(result_f) {}

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

/*!
 * \brief AsyncFuncResultの結果をセットする
 */
struct AsyncFuncResultSetter : Field {
    std::promise<bool> started;
    std::promise<ValAdaptor> result;
    AsyncFuncResultSetter() = default;
    explicit AsyncFuncResultSetter(const Field &base)
        : Field(base), started(), result() {}
    void setStarted(bool is_started) {
        started.set_value(is_started);
        if (!is_started) {
            try {
                throw FuncNotFound(*this);
            } catch (...) {
                result.set_exception(std::current_exception());
            }
        }
    }
    void setResult(ValAdaptor result_val) { result.set_value(result_val); }
    void setResultException(std::exception_ptr e) { result.set_exception(e); }
};

class FuncCallHandle {
    struct HandleData {
        const std::vector<ValAdaptor> args_;
        std::vector<wcfMultiVal> c_args_;
        std::promise<ValAdaptor> result_;
        HandleData(const std::vector<ValAdaptor> &args,
                   std::promise<ValAdaptor> &&result)
            : args_(args), c_args_(args.size()), result_(std::move(result)) {
            for (std::size_t i = 0; i < args_.size(); i++) {
                c_args_[i].as_int = args_[i];
                c_args_[i].as_double = args_[i];
                c_args_[i].as_str =
                    static_cast<const std::string &>(args_[i]).c_str();
            }
        }
    };
    std::shared_ptr<HandleData> handle_data_;

  public:
    FuncCallHandle() = default;
    FuncCallHandle(const std::vector<ValAdaptor> &args,
                   std::promise<ValAdaptor> &&result)
        : handle_data_(std::make_shared<HandleData>(args, std::move(result))) {}
    /*!
     * \brief 関数の引数を取得する
     *
     */
    const std::vector<ValAdaptor> &args() const {
        if (handle_data_) {
            return handle_data_->args_;
        } else {
            throw std::runtime_error("FuncCallHandle does not have valid "
                                     "pointer to function call");
        }
    }
    /*!
     * \brief 関数の引数を取得する
     * \since ver1.7
     *
     */
    const wcfMultiVal *cArgs() const {
        if (handle_data_) {
            return handle_data_->c_args_.data();
        } else {
            throw std::runtime_error("FuncCallHandle does not have valid "
                                     "pointer to function call");
        }
    }
    /*!
     * \brief 関数の結果を送信する
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void respond(ValAdaptor value = "") {
        if (handle_data_) {
            handle_data_->result_.set_value(value);
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
        if (handle_data_) {
            try {
                throw std::runtime_error(message);
            } catch (const std::runtime_error &) {
                handle_data_->result_.set_exception(std::current_exception());
            }
        } else {
            throw std::runtime_error("FuncCallHandle does not have valid "
                                     "pointer to function call");
        }
    }
};
} // namespace WEBCFACE_NS

#pragma once
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include <cstdint>
#include <eventpp/callbacklist.h>
#include "field.h"
#include "common/val.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN

class Member;

/*!
 * \brief Funcの実行ができなかった場合発生する例外
 *
 * (ValueやTextで参照先が見つからなかった場合はこれではなく単にnulloptが返る)
 *
 */
struct WEBCFACE_DLL FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const Common::FieldBase &base)
        : std::runtime_error("member(\"" + Encoding::decode(base.member_) +
                             "\")" + ".func(\"" +
                             Encoding::decode(base.field_) + "\") is not set") {
    }
};

/*!
 * \brief 非同期で実行した関数の実行結果を表す。
 *
 * 結果はshared_futureのget()で得られる。
 *
 * リモートから呼び出しメッセージが送られてきた時非同期で実行して結果を送り返すのにもこれを利用する
 *
 */
class WEBCFACE_DLL AsyncFuncResult : Field {
    std::size_t caller_id;
    std::shared_ptr<eventpp::CallbackList<void(bool)>> started_event;
    std::shared_ptr<eventpp::CallbackList<void(std::shared_future<ValAdaptor>)>>
        result_event;

  public:
    friend class Func;
    AsyncFuncResult() = default;
    AsyncFuncResult(const Field &base, std::size_t caller_id,
                    const std::shared_future<bool> &started_f,
                    const std::shared_future<ValAdaptor> &result_f,
                    const decltype(started_event) &started_event,
                    const decltype(result_event) &result_event)
        : Field(base), caller_id(caller_id), started_event(started_event),
          result_event(result_event), started(started_f), result(result_f) {}

    /*!
     * \brief リモートに呼び出しメッセージが到達したときに値が入る
     *
     * 実行開始したらtrue, 呼び出しに失敗したらfalseが返る。
     * falseの場合resultにFuncNotFound例外が入る
     *
     */
    std::shared_future<bool> started;
    /*!
     * \brief 関数の実行が完了した時戻り値が入る
     *
     * 例外が発生した場合例外が入る
     *
     * ver1.12〜: 例外はresultにexceptionとして格納されるが、
     * そのエラーメッセージにはutf-8ではないstringが使われる
     *
     */
    std::shared_future<ValAdaptor> result;

    /*!
     * \brief リモートに呼び出しメッセージが到達したときに発生するイベント
     * \since ver1.11
     */
    eventpp::CallbackList<void(bool)> &onStarted() const {
        if (!started_event) {
            throw std::runtime_error("started event is null");
        }
        return *started_event;
    }
    /*!
     * \brief 関数の実行が完了した時発生するイベント
     * \since ver1.11
     */
    eventpp::CallbackList<void(std::shared_future<ValAdaptor>)> &
    onResult() const {
        if (!result_event) {
            throw std::runtime_error("result event is null");
        }
        return *result_event;
    }
    using Field::member;
    using Field::name;
};
auto &operator<<(std::basic_ostream<char> &os, const AsyncFuncResult &data);

/*!
 * \brief AsyncFuncResultの結果をセットする
 *
 */
struct AsyncFuncResultSetter : Field {
  private:
    std::promise<bool> started;
    std::promise<ValAdaptor> result;

  public:
    std::shared_future<bool> started_f;
    std::shared_future<ValAdaptor> result_f;
    std::shared_ptr<eventpp::CallbackList<void(bool)>> started_event;
    std::shared_ptr<eventpp::CallbackList<void(std::shared_future<ValAdaptor>)>>
        result_event;
    AsyncFuncResultSetter() = default;
    explicit AsyncFuncResultSetter(const Field &base)
        : Field(base), started(), result(),
          started_f(started.get_future().share()),
          result_f(result.get_future().share()),
          started_event(
              std::make_shared<decltype(started_event)::element_type>()),
          result_event(
              std::make_shared<decltype(result_event)::element_type>()) {}
    void setStarted(bool is_started) {
        started.set_value(is_started);
        started_event->operator()(is_started);
        if (!is_started) {
            try {
                throw FuncNotFound(*this);
            } catch (...) {
                setResultException(std::current_exception());
            }
        }
    }
    void setResult(const ValAdaptor &result_val) {
        result.set_value(result_val);
        result_event->operator()(result_f);
    }
    void setResultException(const std::exception_ptr &e) {
        result.set_exception(e);
        result_event->operator()(result_f);
    }
};

class FuncCallHandle {
    struct HandleData {
        const std::vector<ValAdaptor> args_;
        std::variant<int, std::vector<wcfMultiVal>, std::vector<wcfMultiValW>>
            c_args_;
        std::promise<ValAdaptor> result_;
        HandleData(const std::vector<ValAdaptor> &args,
                   std::promise<ValAdaptor> &&result)
            : args_(args), c_args_(0), result_(std::move(result)) {}

        /*!
         * c_args_ をwcfMultiValの配列またはwcfMultiValWの配列で初期化
         *
         */
        template <std::size_t v_index, typename CVal>
        std::vector<CVal> &initCArgs();
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
     * 引数データを表すwcfMultiValの配列を構築 (createHandle() 用)
     * \since ver1.7(for char), ver1.12(for wchar_t)
     */
    template <typename CharT>
    const auto *cArgs() const {
        if (handle_data_) {
            if constexpr (std::is_same_v<CharT, char>) {
                auto &c_args = handle_data_->initCArgs<1, wcfMultiVal>();
                return c_args.data();
            } else {
                auto &c_args = handle_data_->initCArgs<2, wcfMultiValW>();
                return c_args.data();
            }
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
    template <typename T>
        requires std::constructible_from<ValAdaptor, T>
    void respond(T value) {
        if (handle_data_) {
            handle_data_->result_.set_value(ValAdaptor(value));
        } else {
            throw std::runtime_error("FuncCallHandle does not have valid "
                                     "pointer to function call");
        }
    }
    /*!
     * \brief 空の値を関数の結果として送信する
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void respond() { respond(ValAdaptor()); }
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
    /*!
     * \brief 関数の結果を例外として送信する (wstring)
     * \since ver1.12
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void reject(const std::wstring &message) {
        reject(Encoding::decode(Encoding::encodeW(message)));
    }
};

extern template std::vector<wcfMultiVal> &
FuncCallHandle::HandleData::initCArgs<1, wcfMultiVal>();
extern template std::vector<wcfMultiValW> &
FuncCallHandle::HandleData::initCArgs<2, wcfMultiValW>();

WEBCFACE_NS_END

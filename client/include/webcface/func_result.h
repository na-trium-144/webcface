#pragma once
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include <cstdint>
#include "field.h"
#include "webcface/encoding/val_adaptor.h"
#include <webcface/common/def.h>
#include <webcface/c_wcf/def_types.h>

WEBCFACE_NS_BEGIN

class Member;

/*!
 * \brief Funcの実行ができなかった場合発生する例外
 *
 * (ValueやTextで参照先が見つからなかった場合はこれではなく単にnulloptが返る)
 *
 */
struct WEBCFACE_DLL FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const FieldBase &base);
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
    std::shared_ptr<std::function<void(bool)>> started_event;
    std::shared_ptr<std::function<void(std::shared_future<ValAdaptor>)>>
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
     * ver2.0〜: 例外はresultにexceptionとして格納されるが、
     * そのエラーメッセージにはutf-8ではないstringが使われる
     *
     */
    std::shared_future<ValAdaptor> result;

    /*!
     * \brief
     * リモートに呼び出しメッセージが到達したときに呼び出すコールバックを設定
     * \since ver2.0
     */
    AsyncFuncResult &onStarted(std::function<void(bool)> callback);
    /*!
     * \brief 関数の実行が完了した時呼び出すコールバックを設定
     * \since ver2.0
     * \todo 排他制御をしてない
     */
    AsyncFuncResult &
    onResult(std::function<void(std::shared_future<ValAdaptor>)> callback);

    using Field::member;
    using Field::name;
};
WEBCFACE_DLL std::ostream &operator<<(std::ostream &os,
                                      const AsyncFuncResult &r);

/*!
 * \brief AsyncFuncResultの結果をセットする
 *
 */
struct WEBCFACE_DLL AsyncFuncResultSetter : Field {
  private:
    std::promise<bool> started;
    std::promise<ValAdaptor> result;

  public:
    std::shared_future<bool> started_f;
    std::shared_future<ValAdaptor> result_f;
    std::shared_ptr<std::function<void(bool)>> started_event;
    std::shared_ptr<std::function<void(std::shared_future<ValAdaptor>)>>
        result_event;
    AsyncFuncResultSetter() = default;
    explicit AsyncFuncResultSetter(const Field &base);
    void setStarted(bool is_started);
    void setResult(const ValAdaptor &result_val);
    void setResultException(const std::exception_ptr &e);
};

class WEBCFACE_DLL FuncCallHandle {
    struct WEBCFACE_DLL HandleData {
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

    static std::runtime_error &invalidHandle();

  public:
    FuncCallHandle() = default;
    FuncCallHandle(const std::vector<ValAdaptor> &args,
                   std::promise<ValAdaptor> &&result)
        : handle_data_(std::make_shared<HandleData>(args, std::move(result))) {}
    /*!
     * \brief 関数の引数を取得する
     *
     */
    const std::vector<ValAdaptor> &args() const;
    /*!
     * 引数データを表すwcfMultiValの配列を構築 (createHandle() 用)
     * \since ver1.7
     */
    const wcfMultiVal *cArgs() const;
    /*!
     * 引数データを表すwcfMultiValの配列を構築 (createHandle() 用)
     * \since ver2.0
     */
    const wcfMultiValW *cWArgs() const;
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
            throw invalidHandle();
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
    void reject(const std::string &message);
    /*!
     * \brief 関数の結果を例外として送信する (wstring)
     * \since ver2.0
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void reject(const std::wstring &message);
};

WEBCFACE_NS_END

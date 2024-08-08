#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <optional>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include "field.h"
#include "webcface/encoding/val_adaptor.h"
#include "webcface/c_wcf/def_types.h"

WEBCFACE_NS_BEGIN
namespace internal {
struct PromiseData;
} // namespace internal

/*!
 * \brief Funcの実行ができなかった場合発生する例外
 *
 * ValueやTextで参照先が見つからなかった場合はこれではなく単にnulloptが返る
 *
 * MSVCではexceptionをexportしないほうがよいらしくC4275警告を出すが、
 * Macではexportしないとcatchできなくなる
 *
 */
struct WEBCFACE_DLL FuncNotFound : public std::runtime_error {
    explicit FuncNotFound(const FieldBase &base);
};

/*!
 * \brief 非同期で実行した関数の実行結果を取得するインタフェース。
 *
 * ver1.11まではAsyncFuncResult、ver2.0からPromiseに名前変更
 *
 */
class WEBCFACE_DLL Promise : Field {
    std::shared_ptr<internal::PromiseData> data;

  public:
    Promise(const Field &base,
            const std::shared_ptr<internal::PromiseData> &data,
            const std::shared_future<bool> &started,
            const std::shared_future<ValAdaptor> &result);

    using Field::member;
    using Field::name;
    using Field::nameW;

    /*!
     * \brief リモートに呼び出しメッセージが到達したときに値が入る
     *
     * * 実行開始したらtrue, 呼び出しに失敗したらfalseが返る。
     * falseの場合resultにFuncNotFound例外が入る
     *
     * \deprecated ver2.0〜 reached(), found(), waitReach() を推奨。
     * started.get(), started.wait() 使用時の注意はwaitReach()を参照。
     *
     */
    [[deprecated("use reached(), found() or waitReach()")]]
    std::shared_future<bool> started;
    /*!
     * \brief 関数の実行が完了した時戻り値が入る
     *
     * * 例外が発生した場合例外が入る
     * * ver2.0〜: 例外はresultにexceptionとして格納されるが、
     * そのエラーメッセージにはutf-8ではないstringが使われる
     *
     * \deprecated ver2.0〜 finished(), response(), rejection(), waitFinish()
     * を推奨。 result.get(), result.wait() 使用時の注意はwaitFinish()を参照。
     *
     */
    [[deprecated("use finished(), response(), rejection(), or waitFinish()")]]
    std::shared_future<ValAdaptor> result;

  private:
    void waitReachImpl(std::optional<std::chrono::microseconds> timeout) const;
    void waitFinishImpl(std::optional<std::chrono::microseconds> timeout) const;

  public:
    /*!
     * \brief リモートに呼び出しメッセージが到達したかどうかを返す
     * \since ver2.0
     *
     */
    bool reached() const;
    /*!
     * \brief 呼び出した関数がリモートに存在するか(=実行が開始されたか)を返す
     * \since ver2.0
     *
     * * trueの場合、関数の実行が開始されている(はず)。
     * * 関数がリモートに存在しないなど呼び出しが失敗した場合falseを返し、
     * また finished() はtrue, rejection() もエラーメッセージを返す。
     * * まだリモートに呼び出しメッセージが到達していない場合にもfalseを返す。
     * この場合 reached() がfalse, また finished() もfalse。
     *
     */
    bool found() const;
    /*!
     * \brief リモートに呼び出しメッセージが到達するまで待機
     * \since ver2.0
     *
     * * reached() がtrueになるまで待機する。
     * * Client::sync() を呼ぶのとは別のスレッドで使用することを想定している。
     * 呼び出しが成功したかどうかの情報の受信は Client::sync() で行われるため、
     * この関数を使用して待機している間に Client::sync()
     * が呼ばれていないとデッドロックしてしまうので注意。
     *
     * \sa waitReachFor(), waitReachUntil(), onReached()
     */
    void waitReach() const { waitReachImpl(std::nullopt); }
    /*!
     * \brief リモートに呼び出しメッセージが到達するまで待機
     * \since ver2.0
     *
     * * reached() がtrueになるか、timeoutが経過するまで待機する。
     *
     * \sa waitReach(), waitReachUntil(), onReached()
     */
    void waitReachFor(std::chrono::microseconds timeout) const {
        waitReachImpl(timeout);
    }
    /*!
     * \brief リモートに呼び出しメッセージが到達するまで待機
     * \since ver2.0
     *
     * * reached() がtrueになるか、timeoutが経過するまで待機する。
     *
     * \sa waitReach(), waitReachFor(), onReached()
     */
    template <typename Clock, typename Duration>
    void
    waitReachUntil(std::chrono::time_point<Clock, Duration> timeout) const {
        waitReachImpl(std::chrono::duration_cast<std::chrono::microseconds>(
            timeout - Clock::now()));
    }

    /*!
     * \brief 関数の実行が完了したかどうかを返す
     * \since ver2.0
     *
     */
    bool finished() const;
    /*!
     * \brief 関数の実行が完了した場合その戻り値を返す
     * \since ver2.0
     *
     * まだ完了していない場合、または関数がエラーを返した場合、
     * std::nullopt を返す
     *
     */
    std::optional<ValAdaptor> response() const;
    /*!
     * \brief 関数の実行がエラーになった場合そのエラーメッセージを返す
     * \since ver2.0
     *
     * まだ完了していない場合、または関数がエラーではなく戻り値を返した場合、
     * std::nullopt を返す
     *
     */
    std::optional<std::string> rejection() const;
    /*!
     * \brief 関数の実行がエラーになった場合そのエラーメッセージを返す (wstring)
     * \since ver2.0
     * \sa rejection()
     */
    std::optional<std::wstring> rejectionW() const;

    /*!
     * \brief 関数の実行が完了するまで待機
     * \since ver2.0
     *
     * * finished() がtrueになるまで待機する。
     * * Client::sync() を呼ぶのとは別のスレッドで使用することを想定している。
     * 関数実行が完了したかどうかの情報の受信は Client::sync() で行われるため、
     * この関数を使用して待機している間に Client::sync()
     * が呼ばれていないとデッドロックしてしまうので注意。
     *
     * \sa waitFinishFor(), waitFinishUntil(), onFinished()
     */
    void waitFinish() const { waitFinishImpl(std::nullopt); }
    /*!
     * \brief 関数の実行が完了するまで待機
     * \since ver2.0
     *
     * * finished() がtrueになるか、timeoutが経過するまで待機する。
     *
     * \sa waitFinish(), waitFinishUntil(), onFinished()
     */
    void waitFinishFor(std::chrono::microseconds timeout) const {
        waitFinishImpl(timeout);
    }
    /*!
     * \brief 関数の実行が完了するまで待機
     * \since ver2.0
     *
     * * finished() がtrueになるか、timeoutが経過するまで待機する。
     *
     * \sa waitFinish(), waitFinishFor(), onFinished()
     */
    template <typename Clock, typename Duration>
    void
    waitFinishUntil(std::chrono::time_point<Clock, Duration> timeout) const {
        waitFinishImpl(std::chrono::duration_cast<std::chrono::microseconds>(
            timeout - Clock::now()));
    }

    /*!
     * \brief
     * リモートに呼び出しメッセージが到達したときに呼び出すコールバックを設定
     * \since ver2.0
     *
     * * コールバックの引数にはこのPromiseオブジェクトが渡される。
     * * コールバックは Client::sync() のスレッドで実行され、
     * この関数が完了するまで他のデータの受信処理はブロックされる。
     * * すでにreached()がtrueの場合はこのスレッドで即座にcallbackが呼ばれる。
     *
     */
    Promise &onReached(std::function<void WEBCFACE_CALL_FP(Promise)> callback);
    /*!
     * \brief 関数の実行が完了した時呼び出すコールバックを設定
     * \since ver2.0
     *
     * * コールバックの引数にはこのPromiseオブジェクトが渡される。
     * * コールバックは Client::sync() のスレッドで実行され、
     * この関数が完了するまで他のデータの受信処理はブロックされる。
     * * すでにfinished()がtrueの場合はこのスレッドで即座にcallbackが呼ばれる。
     *
     */
    Promise &onFinished(std::function<void WEBCFACE_CALL_FP(Promise)> callback);
};
using AsyncFuncResult = Promise;

/*!
 * \brief 呼び出された関数の引数の取得と戻り値のセットをするインタフェース
 * \since ver1.5
 *
 * ver1.11までFuncCallHandle, ver2.0からCallHandleに名前変更
 *
 */
class WEBCFACE_DLL CallHandle : Field {
    std::shared_ptr<internal::PromiseData> data;

    static std::runtime_error &WEBCFACE_CALL invalidHandle();

  public:
    CallHandle() = default;
    CallHandle(const Field &base,
               const std::shared_ptr<internal::PromiseData> &data);

    friend class Func;

    using Field::name;
    using Field::nameW;

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
     * 引数データを表すwcfMultiValWの配列を構築 (createHandle() 用)
     * \since ver2.0
     */
    const wcfMultiValW *cWArgs() const;

  private:
    /*!
     * \since ver2.0
     */
    void respond(const ValAdaptor &value) const;
    /*!
     * \since ver2.0
     */
    void reject(const ValAdaptor &message) const;

  public:
    /*!
     * \brief 関数の結果を送信する
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    template <typename T>
    void respond(const T &value) const {
        respond(ValAdaptor(value));
    }
    /*!
     * \brief 空の値を関数の結果として送信する
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void respond() const { respond(ValAdaptor()); }

    /*!
     * \brief 関数の結果を例外として送信する
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void reject(std::string_view message) const { reject(ValAdaptor(message)); }
    /*!
     * \brief 関数の結果を例外として送信する (wstring)
     * \since ver2.0
     *
     * * 2回呼ぶと std::future_error を投げる
     * * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     *
     */
    void reject(std::wstring_view message) const {
        reject(ValAdaptor(message));
    }
};
using FuncCallHandle = CallHandle;

WEBCFACE_NS_END

#pragma once
#include "webcface/common/val_adaptor_vec.h"
#include "webcface/exception.h"
#include <chrono>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <optional>
#include <memory>
#include <functional>
#include "field.h"
#include "webcface/common/val_adaptor.h"
#include "webcface/c_wcf/def_types.h"

WEBCFACE_NS_BEGIN
namespace internal {
struct PromiseData;
struct FuncInfo;
struct ClientData;
} // namespace internal

/*!
 * \brief 非同期で実行した関数の実行結果を取得するインタフェース。
 *
 * ver1.11まではAsyncFuncResult、ver2.0からPromiseに名前変更
 *
 */
class WEBCFACE_DLL Promise : Field {
    std::shared_ptr<internal::PromiseData> data;

    static SanityError WEBCFACE_CALL invalidPromise();

  public:
    Promise(const Field &base,
            const std::shared_ptr<internal::PromiseData> &data);

    using Field::member;
    using Field::name;
    using Field::nameW;

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
     * * onReached
     * にコールバックが設定されている場合そのコールバックの完了も待機する。
     * * Client::sync() を呼ぶのとは別のスレッドで使用することを想定している。
     * 呼び出しが成功したかどうかの情報の受信は Client::sync() で行われるため、
     * この関数を使用して待機している間に Client::sync()
     * が呼ばれていないとデッドロックしてしまうので注意。
     *
     * \sa waitReachFor(), waitReachUntil(), onReached()
     */
    Promise &waitReach() {
        waitReachImpl(std::nullopt);
        return *this;
    }
    /*!
     * \brief リモートに呼び出しメッセージが到達するまで待機
     * \since ver2.0
     *
     * * waitReach()と同じだが、timeoutが経過したらreturnする。
     *
     * \sa waitReach(), waitReachUntil(), onReached()
     */
    Promise &waitReachFor(std::chrono::microseconds timeout) {
        waitReachImpl(timeout);
        return *this;
    }
    /*!
     * \brief リモートに呼び出しメッセージが到達するまで待機
     * \since ver2.0
     *
     * * waitReach()と同じだが、timeoutが経過したらreturnする。
     *
     * \sa waitReach(), waitReachFor(), onReached()
     */
    template <typename Clock, typename Duration>
    Promise &waitReachUntil(std::chrono::time_point<Clock, Duration> timeout) {
        waitReachImpl(std::chrono::duration_cast<std::chrono::microseconds>(
            timeout - Clock::now()));
        return *this;
    }

    /*!
     * \brief 関数の実行が完了したかどうかを返す
     * \since ver2.0
     *
     * finishedがtrueの場合、 response() または rejection() (rejectionW())
     * のどちらかに結果が入る。
     *
     */
    bool finished() const;
    /*!
     * \brief 関数がエラーになったかどうかを返す
     * \since ver2.0
     *
     * * trueの場合、rejection()にエラーメッセージが入る。
     * * 実行が完了していてfalseの場合は、
     * response()に戻り値が入る。
     * * まだ実行が完了していない場合falseを返す。
     *
     */
    bool isError() const;
    /*!
     * \brief 関数の実行が完了した場合その戻り値を返す
     * \since ver2.0
     *
     * ver3.0〜 ValAdptorVectorに変更
     */
    ValAdaptorVector response() const;
    /*!
     * \brief 関数の実行がエラーになった場合そのエラーメッセージを返す
     * \since ver2.0
     *
     * ver2.10〜 StringView型に変更
     *
     */
    StringView rejection() const;
    /*!
     * \brief 関数の実行がエラーになった場合そのエラーメッセージを返す (wstring)
     * \since ver2.0
     * \sa rejection()
     *
     * ver2.10〜 WStringView型に変更
     *
     */
    WStringView rejectionW() const;

    /*!
     * \brief 関数の実行が完了するまで待機
     * \since ver2.0
     *
     * * finished() がtrueになるまで待機する。
     * * onFinished
     * にコールバックが設定されている場合そのコールバックの完了も待機する。
     * * Client::sync() を呼ぶのとは別のスレッドで使用することを想定している。
     * 関数実行が完了したかどうかの情報の受信は Client::sync() で行われるため、
     * この関数を使用して待機している間に Client::sync()
     * が呼ばれていないとデッドロックしてしまうので注意。
     *
     * \sa waitFinishFor(), waitFinishUntil(), onFinished()
     */
    Promise &waitFinish() {
        waitFinishImpl(std::nullopt);
        return *this;
    }
    /*!
     * \brief 関数の実行が完了するまで待機
     * \since ver2.0
     *
     * * waitFinish()と同じだが、timeoutが経過したらreturnする。
     *
     * \sa waitFinish(), waitFinishUntil(), onFinished()
     */
    Promise &waitFinishFor(std::chrono::microseconds timeout) {
        waitFinishImpl(timeout);
        return *this;
    }
    /*!
     * \brief 関数の実行が完了するまで待機
     * \since ver2.0
     *
     * * waitFinish()と同じだが、timeoutが経過したらreturnする。
     *
     * \sa waitFinish(), waitFinishFor(), onFinished()
     */
    template <typename Clock, typename Duration>
    Promise &waitFinishUntil(std::chrono::time_point<Clock, Duration> timeout) {
        waitFinishImpl(std::chrono::duration_cast<std::chrono::microseconds>(
            timeout - Clock::now()));
        return *this;
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
     * \param callback 引数にPromiseをとる関数
     *
     */
    Promise &onReach(std::function<void WEBCFACE_CALL_FP(Promise)> callback);
    /*!
     * \brief
     * リモートに呼び出しメッセージが到達したときに呼び出すコールバックを設定
     * \since ver2.0
     *
     * * コールバックは Client::sync() のスレッドで実行され、
     * この関数が完了するまで他のデータの受信処理はブロックされる。
     * * すでにreached()がtrueの場合はこのスレッドで即座にcallbackが呼ばれる。
     *
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    Promise &onReach(F callback) {
        return onReach(
            [callback = std::move(callback)](const Promise &) { callback(); });
    }
    /*!
     * \brief 関数の実行が完了した時呼び出すコールバックを設定
     * \since ver2.0
     *
     * * コールバックの引数にはこのPromiseオブジェクトが渡される。
     * * コールバックは Client::sync() のスレッドで実行され、
     * この関数が完了するまで他のデータの受信処理はブロックされる。
     * * すでにfinished()がtrueの場合はこのスレッドで即座にcallbackが呼ばれる。
     *
     * \param callback 引数にPromiseをとる関数
     *
     */
    Promise &onFinish(std::function<void WEBCFACE_CALL_FP(Promise)> callback);
    /*!
     * \brief 関数の実行が完了した時呼び出すコールバックを設定
     * \since ver2.0
     *
     * * コールバックの引数にはこのPromiseオブジェクトが渡される。
     * * コールバックは Client::sync() のスレッドで実行され、
     * この関数が完了するまで他のデータの受信処理はブロックされる。
     * * すでにfinished()がtrueの場合はこのスレッドで即座にcallbackが呼ばれる。
     *
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    Promise &onFinish(F callback) {
        return onFinish(
            [callback = std::move(callback)](const Promise &) { callback(); });
    }
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

    static SanityError WEBCFACE_CALL invalidHandle();

  public:
    CallHandle() = default;
    CallHandle(const Field &base,
               const std::shared_ptr<internal::PromiseData> &data);

    friend class Func;
    friend class FuncListener;
    friend struct internal::FuncInfo;
    friend struct internal::ClientData;

    using Field::name;
    using Field::nameW;

    /*!
     * \brief 関数の引数を取得する
     *
     * ver2.10〜 ValAdaptorVectorに変更
     *
     */
    const std::vector<ValAdaptorVector> &args() const;
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

    /*!
     * \since ver3.0
     */
    void respond(const ValAdaptorVector &value) const;
    /*!
     * \since ver2.0
     */
    void reject(const ValAdaptor &message) const;
    /*!
     * \since ver2.0
     */
    void reach(bool found) const;

    /*!
     * \brief 引数の数をチェックする
     * \since ver2.0
     *
     * args().size() != expected
     * ならエラーメッセージとともにrejectし、falseを返す
     *
     * \return 引数の個数がexpectedと一致していたらtrue
     *
     */
    bool assertArgsNum(std::size_t expected) const;

    /*!
     * \brief 関数の結果を送信する
     *
     * * ver1.11まで: 2回呼ぶと std::future_error を投げ、
     * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     * * ver2.0から: respondable() がfalseの場合 std::runtime_error を投げる
     *
     */
    template <typename T>
    void respond(const T &value) const {
        respond(ValAdaptorVector(value));
    }
    /*!
     * \since ver2.10
     */
    void respond(StringInitializer value) const {
        respond(ValAdaptorVector(std::move(value)));
    }
    /*!
     * \brief 空の値を関数の結果として送信する
     *
     * * ver1.11まで: 2回呼ぶと std::future_error を投げ、
     * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     * * ver2.0から: respondable() がfalseの場合 std::runtime_error を投げる
     *
     */
    void respond() const { respond(ValAdaptorVector()); }

    /*!
     * \brief 関数の結果を例外として送信する
     *
     * * ver1.11まで: 2回呼ぶと std::future_error を投げ、
     * このHandleがデフォルト構築されていた場合 std::runtime_error を投げる
     * * ver2.0から: respondable() がfalseの場合 std::runtime_error を投げる
     * * ver2.0〜 wstring対応、ver2.10〜 String型に変更
     *
     */
    void reject(StringInitializer message) const {
        reject(ValAdaptor(std::move(message)));
    }

    /*!
     * \brief respond()またはreject()が可能かどうかを返す
     * \since ver2.0
     * \return このhandleが有効(デフォルト構築などではない)、
     * かつまだrespond()もreject()もしていない状態ならtrue
     *
     */
    bool respondable() const;
};
using FuncCallHandle = CallHandle;

WEBCFACE_NS_END

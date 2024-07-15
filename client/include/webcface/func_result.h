#pragma once
#include <ostream>
#include <string>
#include <future>
#include <memory>
#include <stdexcept>
#include "field.h"
#include "webcface/encoding/val_adaptor.h"
#include "webcface/common/def.h"
#include "webcface/c_wcf/def_types.h"

WEBCFACE_NS_BEGIN
namespace internal {
class AsyncFuncState;
}

class Member;

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
 * \brief 非同期で実行した関数の実行結果を表す。
 *
 * 結果はshared_futureのget()で得られる。
 *
 */
class WEBCFACE_DLL AsyncFuncResult : Field {
    std::shared_ptr<internal::AsyncFuncState> state;

  public:
    AsyncFuncResult(const Field &base,
                    const std::shared_ptr<internal::AsyncFuncState> &state,
                    const std::shared_future<bool> &started,
                    const std::shared_future<ValAdaptor> &result)
        : Field(base), state(state), started(started), result(result) {}

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
     *
     * すでにstartedに値が入っている場合は即座にcallbackが呼ばれる。
     *
     */
    AsyncFuncResult &
    onStarted(std::function<void WEBCFACE_CALL(bool)> callback);
    /*!
     * \brief 関数の実行が完了した時呼び出すコールバックを設定
     * \since ver2.0
     *
     * すでにresultに値または例外が入っている場合は即座にcallbackが呼ばれる。
     *
     */
    AsyncFuncResult &
    onResult(std::function<void WEBCFACE_CALL(std::shared_future<ValAdaptor>)>
                 callback);

    using Field::member;
    using Field::name;
};
WEBCFACE_DLL std::ostream &WEBCFACE_CALL operator<<(std::ostream &os,
                                                    const AsyncFuncResult &r);

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

    static std::runtime_error &WEBCFACE_CALL invalidHandle();

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

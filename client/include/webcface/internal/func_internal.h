#pragma once
#include <condition_variable>
#include <unordered_map>
#include <mutex>
#include "webcface/func_result.h"
#include "webcface/func.h"

WEBCFACE_NS_BEGIN
namespace message {
struct FuncInfo;
struct Call;
} // namespace message

namespace internal {
/*!
 * \brief PromiseとCallHandleのデータを保持するクラス
 *
 */
struct PromiseData : public std::enable_shared_from_this<PromiseData> {
    /*!
     * PromiseDataのメソッドはmutexをロックしないので、呼び出し側がロックすること
     * また、eventの中ではunlockされる
     */
    std::mutex m;
    std::condition_variable cond;

    std::function<void(Promise)> reach_event;
    std::function<void(Promise)> finish_event;
    bool reach_event_done = false;
    bool finish_event_done = false;

    bool reached = false;
    bool found = false;
    bool finished = false;
    bool is_error = false;
    ValAdaptorVector response;
    ValAdaptor rejection;

    Field base;
    std::size_t caller_id;
    const std::vector<ValAdaptorVector> args_;
    std::variant<int, std::vector<wcfMultiVal>, std::vector<wcfMultiValW>>
        c_args_;

    /*!
     * startedとstartedEventが両方セットされていればコールバック発動
     */
    void callReachEvent();
    /*!
     * resultとresultEventが両方セットされていればコールバック発動
     */
    void callFinishEvent();

    /*!
     * startedとresultを空の状態で初期化
     */
    explicit PromiseData(const Field &base, std::vector<ValAdaptorVector> &&args,
                         std::size_t caller_id = 0)
        : reach_event(), finish_event(), base(base),
          caller_id(caller_id), args_(std::move(args)) {}

    /*!
     * c_args_ をwcfMultiValの配列またはwcfMultiValWの配列で初期化
     *
     */
    template <std::size_t v_index, typename CVal>
    std::vector<CVal> &initCArgs();

    Promise getter();
    CallHandle setter();

    std::size_t callerId() const { return caller_id; }
};

/*!
 * \brief PromiseDataのリストを保持する。
 *
 * 関数の実行結果が返ってきた時参照する
 * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
 *
 */
class FuncResultStore {
    std::mutex mtx;
    std::unordered_map<std::size_t, std::shared_ptr<PromiseData>> promises;
    std::size_t next_caller_id = 0;

  public:
    /*!
     * \brief 新しいcaller_idを振って新しいAsyncFuncStateを生成しそれを返す
     *
     */
    std::shared_ptr<PromiseData> addResult(const Field &base) {
        std::lock_guard lock(mtx);
        std::size_t caller_id = next_caller_id++;
        auto state = std::make_shared<PromiseData>(
            base, std::vector<ValAdaptorVector>{}, caller_id);
        promises.emplace(caller_id, state);
        return state;
    }
    /*!
     * \brief AsyncFuncStateを取得
     *
     */
    std::shared_ptr<PromiseData> getResult(std::size_t caller_id) {
        std::lock_guard lock(mtx);
        auto it = promises.find(caller_id);
        if (it != promises.end()) {
            return it->second;
        } else {
            throw PromiseError("caller id not found");
        }
    }
    /*!
     * \brief resultを設定し終わったstateを削除
     *
     */
    void removeResult(std::size_t caller_id) {
        std::lock_guard lock(mtx);
        auto it = promises.find(caller_id);
        if (it != promises.end()) {
            promises.erase(it);
        } else {
            throw PromiseError("caller id not found");
        }
    }
};

/*!
 * \brief 関数1つの情報を表す。関数の実体も持つ
 *
 * ver2.4〜: args が空vectorではなくstd::nulloptの場合、
 * 引数の個数が確定していないことを表し、
 * あとから setArgs() で個数を変更することができる
 *
 */
struct FuncInfo : public Field {
    ValType return_type;
    std::optional<std::vector<Arg>> args;
    std::function<Func::FuncType> func_impl;
    int index;

    /*!
     * \brief func_implをこのスレッドで実行
     *
     * 非同期実行する必要のある関数はfunc_impl内(Func::set1()内)でスレッドを建てる
     *
     */
    void run(const CallHandle &handle) {
        if (func_impl) {
            func_impl(handle);
        } else {
            throw SanityError("func_impl is null");
        }
    }

    /*!
     * \brief
     * func_implをこのスレッドで実行し、完了時にCallResultメッセージを送る
     *
     */
    void run(webcface::message::Call &&call);

    FuncInfo();
    FuncInfo(const Field &base, ValType return_type,
             std::optional<std::vector<Arg>> &&args,
             std::function<Func::FuncType> &&func_impl);
    FuncInfo(const message::FuncInfo &m);
    message::FuncInfo toMessage(const SharedString &field);
};

} // namespace internal
WEBCFACE_NS_END

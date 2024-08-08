#pragma once
#include <mutex>
#include "webcface/func_result.h"
#include "webcface/func.h"

WEBCFACE_NS_BEGIN
namespace message {
struct FuncInfo;
}

namespace internal {
/*!
 * \brief PromiseとCallHandleのデータを保持するクラス
 *
 */
struct PromiseData : public std::enable_shared_from_this<PromiseData> {
    std::recursive_mutex m;

    std::function<void(Promise)> reach_event;
    std::function<void(Promise)> finish_event;
    bool reach_event_done = false;
    bool finish_event_done = false;

    bool reached = false;
    bool found = false;
    bool finished = false;
    ValAdaptor response;
    ValAdaptor rejection;

    std::promise<bool> started_p;
    std::promise<ValAdaptor> result_p;
    // std::size_t caller_id;
    std::shared_future<bool> started_f;
    std::shared_future<ValAdaptor> result_f;

    Field base;
    const std::vector<ValAdaptor> args_;
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

  public:
    /*!
     * startedとresultを空の状態で初期化
     */
    explicit PromiseData(const Field &base, std::vector<ValAdaptor> &&args)
        : reach_event(), finish_event(), started_p(), result_p(),
          started_f(started_p.get_future().share()),
          result_f(result_p.get_future().share()), base(base),
          args_(std::move(args)) {}

    /*!
     * c_args_ をwcfMultiValの配列またはwcfMultiValWの配列で初期化
     *
     */
    template <std::size_t v_index, typename CVal>
    std::vector<CVal> &initCArgs();

    Promise getter();
    CallHandle setter();
    // std::size_t callerId() const { return caller_id; }

    /*!
     * reached, found をセットしreachedEventを呼ぶ
     * found=falseならfinishEventも呼ぶ
     */
    void reach(bool found);
    /*!
     * resultをセットしfinishEventを呼ぶ
     */
    void respond(const ValAdaptor &result_val);
    /*!
     * resultをセットしfinishEventを呼ぶ
     */
    void reject(const ValAdaptor &message);

    /*!
     * reachEventをセットしreachEventを呼ぶ
     */
    void setReachEvent(std::function<void(Promise)> &&callback);
    /*!
     * finishEventをセットしfinishEventを呼ぶ
     */
    void setFinishEvent(std::function<void(Promise)> &&callback);
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
        auto state = std::make_shared<PromiseData>(base, caller_id);
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
            throw std::out_of_range("caller id not found");
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
            throw std::out_of_range("caller id not found");
        }
    }
};

/*!
 * \brief 関数1つの情報を表す。関数の実体も持つ
 *
 */
struct FuncInfo {
    ValType return_type;
    std::vector<Arg> args;
    std::function<Func::FuncType> func_impl;

    /*!
     * \brief func_implをこのスレッドで実行
     *
     * 非同期実行する必要のある関数はfunc_impl内(Func::set1()内)でスレッドを建てる
     *
     */
    void run(const CallHandle &handle,
             const std::vector<ValAdaptor> &call_args) {
        if (func_impl) {
            func_impl(handle, call_args);
        } else {
            throw std::runtime_error("func_impl is null");
        }
    }

    FuncInfo() : return_type(ValType::none_), args(), func_impl() {}
    FuncInfo(ValType return_type, std::vector<Arg> args,
             std::function<Func::FuncType> func_impl)
        : return_type(return_type), args(std::move(args)),
          func_impl(std::move(func_impl)) {}
    FuncInfo(const message::FuncInfo &m);
    message::FuncInfo toMessage(const SharedString &field) const;
};

} // namespace internal
WEBCFACE_NS_END

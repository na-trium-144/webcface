#pragma once
#include <mutex>
#include <webcface/func_info.h>
#include <webcface/func_result.h>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace internal {

/*!
 * \brief AsyncFuncResultのデータを保持するクラス
 *
 */
class WEBCFACE_DLL AsyncFuncState
    : public std::enable_shared_from_this<AsyncFuncState> {
    std::function<void(bool)> started_event;
    std::function<void(std::shared_future<ValAdaptor>)> result_event;
    std::promise<bool> started_p;
    std::promise<ValAdaptor> result_p;
    std::size_t caller_id;
    std::shared_future<bool> started_f;
    std::shared_future<ValAdaptor> result_f;
    Field base;

  public:
    /*!
     * startedとresultを空の状態で初期化
     */
    explicit AsyncFuncState(const Field &base, std::size_t caller_id = 0)
        : started_event(), result_event(), started_p(), result_p(),
          caller_id(caller_id), started_f(started_p.get_future().share()),
          result_f(result_p.get_future().share()), base(base) {}
    /*!
     * startedをtrueで, resultにfutureを渡して初期化
     *
     * \todo resultEventは?
     */
    explicit AsyncFuncState(const Field &base,
                            const std::shared_future<ValAdaptor> &result,
                            std::size_t caller_id = 0)
        : started_event(), result_event(), started_p(), result_p(),
          caller_id(caller_id), started_f(started_p.get_future().share()),
          result_f(result), base(base) {
        setStarted(true);
    }

    static std::shared_ptr<AsyncFuncState> notFound(const Field &base);
    static std::shared_ptr<AsyncFuncState>
    running(const Field &base, const std::shared_future<ValAdaptor> &result);
    static std::shared_ptr<AsyncFuncState> error(const Field &base,
                                                 const std::exception_ptr &e);
    static std::shared_ptr<AsyncFuncState> remote(const Field &base,
                                                  std::size_t caller_id);

    AsyncFuncResult getter() {
        return AsyncFuncResult(base, shared_from_this(), started_f, result_f);
    }
    std::size_t callerId() const { return caller_id; }
    auto &startedEvent() { return started_event; }
    auto &resultEvent() { return result_event; }

    void setStarted(bool is_started);
    void setResult(const ValAdaptor &result_val);
    void setResultException(const std::exception_ptr &e);
};

/*!
 * \brief AsyncFuncResultのリストを保持する。
 *
 * 関数の実行結果が返ってきた時参照する
 * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
 *
 */
class FuncResultStore {
    std::mutex mtx;
    std::unordered_map<std::size_t, std::shared_ptr<AsyncFuncState>>
        result_setter;
    std::size_t next_caller_id = 0;

  public:
    /*!
     * \brief 新しいcaller_idを振って新しいAsyncFuncStateを生成しそれを返す
     *
     */
    std::shared_ptr<AsyncFuncState> addResult(const Field &base) {
        std::lock_guard lock(mtx);
        std::size_t caller_id = next_caller_id++;
        auto state = AsyncFuncState::remote(base, caller_id);
        result_setter.emplace(caller_id, state);
        return state;
    }
    /*!
     * \brief AsyncFuncStateを取得
     *
     */
    std::shared_ptr<AsyncFuncState> getResult(std::size_t caller_id) {
        std::lock_guard lock(mtx);
        auto it = result_setter.find(caller_id);
        if (it != result_setter.end()) {
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
        auto it = result_setter.find(caller_id);
        if (it != result_setter.end()) {
            result_setter.erase(it);
        } else {
            throw std::out_of_range("caller id not found");
        }
    }
};

} // namespace internal
WEBCFACE_NS_END

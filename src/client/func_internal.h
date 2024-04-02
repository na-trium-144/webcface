#pragma once
#include <mutex>
#include <vector>
#include <string>
#include <condition_variable>
#include <webcface/func_result.h>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Internal {

/*!
 * \brief AsyncFuncResultのリストを保持する。
 *
 * 関数の実行結果が返ってきた時参照する
 * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
 *
 */
class FuncResultStore {
    std::mutex mtx;
    std::unordered_map<std::size_t, AsyncFuncResultSetter> result_setter;
    std::size_t next_caller_id = 0;

  public:
    /*!
     * \brief 新しいcaller_idを振って新しいAsyncFuncResultを生成しそれを返す
     *
     */
    AsyncFuncResult addResult(const Field &base) {
        std::lock_guard lock(mtx);
        std::size_t caller_id = next_caller_id++;
        result_setter.emplace(caller_id, AsyncFuncResultSetter{base});
        return AsyncFuncResult{
            base, caller_id,
            result_setter.at(caller_id).started.get_future().share(),
            result_setter.at(caller_id).result.get_future().share()};
    }
    /*!
     * \brief promiseを取得
     *
     */
    AsyncFuncResultSetter &resultSetter(std::size_t caller_id) {
        std::lock_guard lock(mtx);
        auto it = result_setter.find(caller_id);
        if (it != result_setter.end()) {
            return it->second;
        } else {
            throw std::out_of_range("caller id not found");
        }
    }
    /*!
     * \brief resultを設定し終わったpromiseを削除
     *
     * 
     */
    void removeResultSetter(std::size_t caller_id) {
        std::lock_guard lock(mtx);
        auto it = result_setter.find(caller_id);
        if (it != result_setter.end()) {
            result_setter.erase(it);
        } else {
            throw std::out_of_range("caller id not found");
        }
    }
};

/*!
 * \brief clientがsync()されたタイミングで実行中の関数を起こす
 * さらにその関数が完了するまで待機する
 *
 */
class FuncOnSync {
    std::mutex call_mtx, return_mtx;
    std::condition_variable call_cond, return_cond;

  public:
    /*!
     * \brief sync()側が関数を起こし完了まで待機
     *
     */
    void sync() {
        std::unique_lock return_lock(return_mtx);
        { std::lock_guard call_lock(call_mtx); }
        call_cond.notify_all();
        return_cond.wait(return_lock);
    }
    /*!
     * \brief 関数側がsync()まで待機
     *
     */
    void wait() {
        std::unique_lock call_lock(call_mtx);
        call_cond.wait(call_lock);
    }
    /*!
     * \brief 関数側が完了を通知
     *
     */
    void done() {
        {
            // sync()側が return_cond.wait() に到達するまでブロック

            std::lock_guard return_lock(return_mtx);
        }
        return_cond.notify_all();
    }
};

} // namespace Internal
WEBCFACE_NS_END

#include <mutex>
#include <vector>
#include <string>
#include <condition_variable>
#include <webcface/func_result.h>

namespace WebCFace::Internal {

//! AsyncFuncResultのリストを保持する。
/*! 関数の実行結果が返ってきた時参照する
 * また、実行するたびに連番を振る必要があるcallback_idの管理にも使う
 */
class FuncResultStore {
    std::mutex mtx;
    std::vector<AsyncFuncResult> results;

  public:
    //! 新しいcaller_idを振って新しいAsyncFuncResultを生成しそれを返す
    AsyncFuncResult &addResult(const std::string &caller, const Field &base) {
        std::lock_guard lock(mtx);
        std::size_t caller_id = results.size();
        results.push_back(AsyncFuncResult{caller_id, caller, base});
        return results.back();
    }
    //! caller_idに対応するresultを返す
    //! 存在しない場合out_of_rangeを投げる
    AsyncFuncResult &getResult(std::size_t caller_id) {
        std::lock_guard lock(mtx);
        return results.at(caller_id);
    }
};

//! clientがsync()されたタイミングで実行中の関数を起こす
//! さらにその関数が完了するまで待機する
class FuncOnSync {
    std::mutex call_mtx, return_mtx;
    std::condition_variable call_cond, return_cond;

  public:
    //! sync()側が関数を起こし完了まで待機
    void sync() {
        std::unique_lock return_lock(return_mtx);
        { std::lock_guard call_lock(call_mtx); }
        call_cond.notify_all();
        return_cond.wait(return_lock);
    }
    //! 関数側がsync()まで待機
    void wait() {
        std::unique_lock call_lock(call_mtx);
        call_cond.wait(call_lock);
    }
    //! 関数側が完了を通知
    void done() {
        {
            // sync()側が return_cond.wait() に到達するまでブロック
            std::lock_guard return_lock(return_mtx);
        }
        return_cond.notify_all();
    }
};

} // namespace WebCFace::Internal
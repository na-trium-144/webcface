#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
inline namespace Common {
//! 排他制御をしたただのキュー
template <typename T>
class Queue {
    std::mutex mtx;
    std::condition_variable cond;
    std::queue<T> que;

  public:
    void push(const T &f) {
        std::lock_guard lock(mtx);
        que.push(f);
        cond.notify_all();
    }
    std::optional<T> pop() {
        std::lock_guard lock(mtx);
        if (!que.empty()) {
            auto c = que.front();
            que.pop();
            return c;
        }
        return std::nullopt;
    }
    std::optional<T> pop(std::chrono::milliseconds d) {
        std::unique_lock lock(mtx);
        if (cond.wait_for(lock, d, [&] { return !que.empty(); })) {
            auto c = que.front();
            que.pop();
            return c;
        }
        return std::nullopt;
    }
    void clear() {
        std::lock_guard lock(mtx);
        std::queue<T>().swap(que);
    }
};
} // namespace Common
WEBCFACE_NS_END

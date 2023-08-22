#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <optional>

namespace WebCFace {
//! 排他制御をしたただのキュー
template <typename T>
class Queue {
    std::mutex mtx;
    std::condition_variable cond;
    std::queue<T> que;

  public:
    void push(const T &f) {
        {
            std::lock_guard lock(mtx);
            que.push(f);
        }
        cond.notify_one();
    }
    template <typename Dur = std::chrono::milliseconds>
    std::optional<T> pop(const Dur &d = std::chrono::milliseconds(0)) {
        std::unique_lock lock(mtx);
        if (cond.wait_for(lock, d, [this] { return !que.empty(); })) {
            auto c = que.front();
            que.pop();
            return c;
        }
        return std::nullopt;
    }
};
} // namespace WebCFace
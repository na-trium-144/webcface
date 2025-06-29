#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <shared_mutex>
#include <mutex>
#include <cassert>
#include <condition_variable>
#include <atomic>
#include <thread>
#include <chrono>
#include "./unlock.h"

WEBCFACE_NS_BEGIN
namespace internal {

template <typename Proxy, typename Lock>
class ScopedLock : public Lock {
    Proxy *p;

  public:
    ScopedLock(Proxy *p) : Lock(p->m), p(p) {}
    [[nodiscard]] auto &get() {
        assert(this->owns_lock());
        return p->data;
    }
    auto *operator->() {
        assert(this->owns_lock());
        return &p->data;
    }
    [[nodiscard]] auto &cond() { return p->cond; }
};

class PollingConditionVariable {
    inline static constexpr std::chrono::microseconds poll_interval{1};
    std::atomic<int> notified;

public:
    PollingConditionVariable(): notified(0){}
    void notify_all(){
        ++notified;
    }
    void wait(std::unique_lock<std::mutex> &lock){
        int notify_target = notified.load();
        ScopedUnlock un(lock);
        while(notified.load() == notify_target){
            std::this_thread::sleep_for(poll_interval);
        }
    }
    template <typename Pred>
    void wait(std::unique_lock<std::mutex> &lock, Pred pred){
        while(!pred()){
            ScopedUnlock un(lock);
            std::this_thread::sleep_for(poll_interval);
        }
    }
    template <typename Clock, typename Duration>
    std::cv_status wait_until(std::unique_lock<std::mutex> &lock, const std::chrono::time_point<Clock, Duration>&abs){
        int notify_target = notified.load();
        ScopedUnlock un(lock);
        while(notified.load() == notify_target){
            if(Clock::now() >= abs){
                return std::cv_status::timeout;
            }
            std::this_thread::sleep_for(poll_interval);
        }
        return std::cv_status::no_timeout;
    }
    template <typename Clock, typename Duration, typename Pred>
    bool wait_until(std::unique_lock<std::mutex> &lock, const std::chrono::time_point<Clock, Duration>&abs, Pred pred){
        while(!pred()){
            if(Clock::now() >= abs){
                return false;
            }
            ScopedUnlock un(lock);
            std::this_thread::sleep_for(poll_interval);
        }
        return true;
    }
    template <typename Rep, typename Period>
    auto wait_for(std::unique_lock<std::mutex> &lock, const std::chrono::duration<Rep, Period>&rel){
        return wait_until(lock, std::chrono::steady_clock::now() + rel);
    }
    template <typename Rep, typename Period, typename Pred>
    auto wait_for(std::unique_lock<std::mutex> &lock, const std::chrono::duration<Rep, Period>&rel, Pred pred){
        return wait_until(lock, std::chrono::steady_clock::now() + rel, pred);
    }
};

/*!
 * \brief RustのMutexのようにデータを保護するクラス
 *
 * ```
 * MutexProxy<T> data;
 *
 * {
 *   auto lock = data.lock();
 *   lock->foo();
 * }
 *
 * {
 *   data.lock()->foo();
 * }
 * ```
 */
template <typename T>
class MutexProxy {
    T data;
    mutable std::mutex m;
    mutable PollingConditionVariable cond;

  public:
    template <typename Proxy, typename Lock>
    friend class ScopedLock;

    [[nodiscard]] ScopedLock<MutexProxy, std::unique_lock<std::mutex>> lock() {
        return this;
    }
    [[nodiscard]] ScopedLock<const MutexProxy, std::unique_lock<std::mutex>>
    const_lock() const {
        return this;
    }
};
template <typename T>
class SharedMutexProxy {
    T data;
    mutable std::shared_mutex m;

  public:
    template <typename Proxy, typename Lock>
    friend class ScopedLock;

    [[nodiscard]] ScopedLock<SharedMutexProxy,
                             std::unique_lock<std::shared_mutex>>
    lock() {
        return this;
    }
    [[nodiscard]] ScopedLock<const SharedMutexProxy,
                             std::shared_lock<std::shared_mutex>>
    shared_lock() const {
        return this;
    }
};

} // namespace internal
WEBCFACE_NS_END

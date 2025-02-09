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
    mutable std::condition_variable cond;

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

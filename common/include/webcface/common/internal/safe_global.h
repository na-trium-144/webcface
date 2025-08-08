#pragma once
#include <cassert>
#include <utility>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace internal {
template <typename T>
class SafeGlobal {
    T obj;
    bool alive;

  public:
    /*!
     * コピーコンストラクタを禁止するため、ArgがSafeGlobalの場合を除外
     *
     */
    template <typename... Args,
              typename std::enable_if_t<
                  (!std::is_same_v<std::decay_t<Args>, SafeGlobal> && ...),
                  std::nullptr_t> = nullptr>
    SafeGlobal(Args &&...args)
        : obj(std::forward<Args>(args)...), alive(true) {}
    SafeGlobal(const SafeGlobal &) = delete;
    SafeGlobal &operator=(const SafeGlobal &) = delete;
    SafeGlobal(SafeGlobal &&) = delete;
    SafeGlobal &operator=(SafeGlobal &&) = delete;
    ~SafeGlobal() { alive = false; }
    operator bool() const { return alive; }
    T &operator*() {
        assert(alive);
        return obj;
    }
    T *operator->() {
        assert(alive);
        return &obj;
    }
};

template <typename T>
SafeGlobal<T> safeGlobal(T obj) {
    return SafeGlobal<T>(std::move(obj));
}

} // namespace internal
WEBCFACE_NS_END

#pragma once
#include "allocator.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace smr {

template <typename T>
using shared_ptr = std::shared_ptr<T>;

template <typename T, typename... Args>
std::shared_ptr<T> make_shared(Args &&...args) {
    return std::allocate_shared<T>(getStaticAllocator(),
                                   std::forward<Args>(args)...);
}

template <typename T>
using unique_ptr = std::unique_ptr<T, SharedResourceDeleter<T>>;

template <typename T, typename... Args>
unique_ptr<T> make_unique(Args &&...args) {
    SharedResourceAllocator<T> alloc;
    T *ptr = alloc.allocate(1);
    try {
        alloc.construct(ptr, std::forward<Args>(args)...);
    } catch (...) {
        alloc.deallocate(ptr, 1);
        throw;
    }
    return unique_ptr<T>(ptr, SharedResourceDeleter<T>(alloc));
}

} // namespace smr
WEBCFACE_NS_END

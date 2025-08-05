#pragma once
#include <webcface/common/smr/allocator.h>

WEBCFACE_NS_BEGIN
namespace smr {

struct PoolResourceContext {
    std::shared_ptr<std::pmr::memory_resource> pool_resource;
    bool initialized = false;

    PoolResourceContext() {
        pool_resource = std::make_shared<std::pmr::synchronized_pool_resource>(
            std::pmr::pool_options(), std::pmr::new_delete_resource());
        initialized = true;
    }
    ~PoolResourceContext() {
        pool_resource.reset();
        initialized = false;
    }
};
SharedResourceAllocator<char> getStaticAllocator() {
    static PoolResourceContext context;
    if (context.initialized) {
        return SharedResourceAllocator<char>(context.pool_resource.get(),
                                             context.pool_resource);
    } else {
        return SharedResourceAllocator<char>(std::pmr::new_delete_resource(),
                                             nullptr);
    }
}

} // namespace smr
WEBCFACE_NS_END
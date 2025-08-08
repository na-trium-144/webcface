#include <webcface/common/smr/allocator.h>
#include <webcface/common/internal/safe_global.h>

WEBCFACE_NS_BEGIN
namespace smr {

SharedResourceAllocator<char> getStaticAllocator() {
    static auto pool_resource = internal::safeGlobal(
        std::make_shared<std::pmr::synchronized_pool_resource>(
            std::pmr::pool_options(), std::pmr::new_delete_resource()));
    if (pool_resource) {
        return SharedResourceAllocator<char>(pool_resource->get(),
                                             *pool_resource);
    } else {
        return SharedResourceAllocator<char>(std::pmr::new_delete_resource(),
                                             nullptr);
    }
}

} // namespace smr
WEBCFACE_NS_END

#pragma once
#include <map>
#include "allocator.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace smr {

template <typename Key, typename T, typename Compare = std::less<Key>>
using map =
    std::map<Key, T, Compare, SharedResourceAllocator<std::pair<const Key, T>>>;

} // namespace smr
WEBCFACE_NS_END

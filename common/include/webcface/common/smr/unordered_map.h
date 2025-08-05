#pragma once
#include <unordered_map>
#include "allocator.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace smr {

template <typename Key, typename T, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
using unordered_map =
    std::unordered_map<Key, T, Hash, KeyEqual,
                       SharedResourceAllocator<std::pair<const Key, T>>>;

} // namespace smr
WEBCFACE_NS_END

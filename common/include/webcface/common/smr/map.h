#pragma once
#include <map>
#include <unordered_map>
#include "allocator.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace smr {

template <typename Key, typename T,
          typename Compare = std::less<Key>,
          typename Allocator = SharedResourceAllocator<std::pair<const Key, T>>>
using map = std::map<Key, T, Compare, Allocator>;

template <typename Key, typename T,
          typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>,
          typename Allocator = SharedResourceAllocator<std::pair<const Key, T>>>
using unordered_map = std::unordered_map<Key, T, Hash, KeyEqual, Allocator>;

}
WEBCFACE_NS_END

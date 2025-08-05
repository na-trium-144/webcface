#pragma once
#include <vector>
#include "allocator.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace smr {

template <typename T>
using vector = std::vector<T, SharedResourceAllocator<T>>;

}
WEBCFACE_NS_END

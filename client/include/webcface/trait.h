#pragma once
#include <cstddef>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace traits {
using TraitOkType = std::nullptr_t;
constexpr std::nullptr_t TraitOk = nullptr;
} // namespace traits
WEBCFACE_NS_END

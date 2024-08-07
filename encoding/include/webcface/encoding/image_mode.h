#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
inline namespace encoding {
enum class ImageColorMode {
    gray = 0,
    bgr = 1,
    bgra = 2,
    rgb = 3,
    rgba = 4,
};
enum class ImageCompressMode {
    raw = 0,
    jpeg = 1,
    webp = 2,
    png = 3,
};

} // namespace encoding
WEBCFACE_NS_END

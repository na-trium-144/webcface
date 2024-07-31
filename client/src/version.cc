#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/version.h"

WEBCFACE_NS_BEGIN

const std::array<int, 3> version = {
    WEBCFACE_VERSION_MAJOR, WEBCFACE_VERSION_MINOR, WEBCFACE_VERSION_REVISION};
const char *version_s = WEBCFACE_VERSION;

WEBCFACE_NS_END

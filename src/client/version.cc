#include <webcface/common/def.h>
#include <webcface/common/version.h>
WEBCFACE_NS_BEGIN
inline namespace Version {
const std::array<int, 3> version = {
    WEBCFACE_VERSION_MAJOR, WEBCFACE_VERSION_MINOR, WEBCFACE_VERSION_REVISION};
const char *version_s = WEBCFACE_VERSION;
} // namespace Version
WEBCFACE_NS_END

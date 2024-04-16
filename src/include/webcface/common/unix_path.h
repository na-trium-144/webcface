#include "def.h"
#include <filesystem>

WEBCFACE_NS_BEGIN
inline namespace Common {
inline std::filesystem::path unixSocketPath(int port) {
#ifdef _WIN32
    return "C:\\ProgramData\\webcface\\" + std::to_string(port) + ".sock";
#else
    return "/tmp/webcface/" + std::to_string(port) + ".sock";
#endif
}
} // namespace Common
WEBCFACE_NS_END

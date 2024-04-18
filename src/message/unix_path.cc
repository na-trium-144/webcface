#include "unix_path.h"

WEBCFACE_NS_BEGIN
namespace Message {
std::filesystem::path unixSocketPath(int port) {
#ifdef _WIN32
    return "C:\\ProgramData\\webcface\\" + std::to_string(port) + ".sock";
#else
    return "/tmp/webcface/" + std::to_string(port) + ".sock";
#endif
}
std::optional<std::filesystem::path> unixSocketPathWSLInterop(int port) {
#ifdef _WIN32
#else
    if (std::filesystem::exists("/proc/sys/fs/binfmt_misc/WSLInterop") &&
        std::filesystem::exists("/mnt/c/ProgramData")) {
        return "/mnt/c/ProgramData/webcface/" + std::to_string(port) + ".sock";
    }
#endif
    return std::nullopt;
}
} // namespace Message
WEBCFACE_NS_END

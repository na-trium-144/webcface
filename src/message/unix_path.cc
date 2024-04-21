#include "unix_path.h"
#ifdef _WIN32
#include <shlobj_core.h>
#endif

WEBCFACE_NS_BEGIN
namespace Message::Path {
std::filesystem::path unixSocketPath(int port) {
#ifdef _WIN32
    wchar_t *fpath;
    SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &fpath);
    return std::filesystem::path(fpath) / "webcface" /
           (std::to_string(port) + ".sock");
#else
    return "/tmp/webcface/" + std::to_string(port) + ".sock";
#endif
}
std::optional<std::filesystem::path>
unixSocketPathWSLInterop([[maybe_unused]] int port) {
#ifdef _WIN32
#else
    if (std::filesystem::exists("/proc/sys/fs/binfmt_misc/WSLInterop") &&
        std::filesystem::exists("/mnt/c/ProgramData")) {
        return "/mnt/c/ProgramData/webcface/" + std::to_string(port) + ".sock";
    }
#endif
    return std::nullopt;
}

void initUnixSocket(const std::filesystem::path &path,
                    std::shared_ptr<spdlog::logger> logger) {
    try {
        std::filesystem::create_directories(path.parent_path());
    } catch (const std::filesystem::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
    try {
        std::filesystem::remove(path);
    } catch (const std::filesystem::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
}
void updateUnixSocketPerms(const std::filesystem::path &path,
                           std::shared_ptr<spdlog::logger> logger) {
    try {
        std::filesystem::permissions(path.parent_path(),
                                     std::filesystem::perms::all);
    } catch (const std::filesystem::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
    try {
        std::filesystem::permissions(path, std::filesystem::perms::all);
    } catch (const std::filesystem::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
}

} // namespace Message::Path
WEBCFACE_NS_END

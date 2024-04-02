#include "dir.h"
#include <filesystem>
#include <array>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <climits>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

WEBCFACE_NS_BEGIN
namespace Server {

// https://stackoverflow.com/questions/50889647/best-way-to-get-exe-folder-path
std::filesystem::path
getExeDir([[maybe_unused]] const std::shared_ptr<spdlog::logger> &logger) {
#ifdef _WIN32
    // Windows specific
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    return std::filesystem::path{szPath}.parent_path();
#elif __APPLE__
    char szPath[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (!_NSGetExecutablePath(szPath, &bufsize)) {
        return std::filesystem::path{szPath}
            .parent_path(); // to finish the folder path with (back)slash
    }
    logger->critical("_NSGetExecutablePath error");
    return {}; // some error
#else
    // Linux specific
    char szPath[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", szPath, PATH_MAX);
    if (count < 0 || count >= PATH_MAX) {
        logger->critical("readlink error");
        return {}; // some error
    }
    szPath[count] = '\0';
    return std::filesystem::path{szPath}.parent_path();
#endif
}
std::string getStaticDir(const std::shared_ptr<spdlog::logger> &logger) {
    auto exe_dir = getExeDir(logger);
    std::array<std::filesystem::path, 8> static_dirs = {
        exe_dir / "dist",
        exe_dir.parent_path() / "dist",
        exe_dir.parent_path() / "webui" / "dist",
        exe_dir.parent_path().parent_path() / "dist",
        exe_dir.parent_path().parent_path() / "webui" / "dist",
        exe_dir.parent_path().parent_path().parent_path() / "dist",
        exe_dir.parent_path().parent_path().parent_path() / "webui" / "dist",
        exe_dir.parent_path() / "share" / "webcface" / "dist",
    };
    for (const auto &dir : static_dirs) {
        try {
            if (std::filesystem::exists(dir / "index.html")) {
                logger->debug("{} found.", (dir / "index.html").string());
                return dir.string();
            }
        } catch (...) {
        }
        logger->debug("{} not found.", (dir / "index.html").string());
    }
    logger->warn("Cannot find webui dist directory.");
    logger->debug("Falling back to <executable_dir>/dist");
    return (exe_dir / "dist").string();
}
std::string getTempDir(const std::shared_ptr<spdlog::logger> &logger) {
    try {
        return std::filesystem::temp_directory_path().string();
    } catch (...) {
        logger->critical("temp_directory_path error");
        return "";
    }
}
} // namespace Server
WEBCFACE_NS_END

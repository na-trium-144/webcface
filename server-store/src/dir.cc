#include "webcface/server/dir.h"
#include <array>

#if WEBCFACE_EXP_FILESYSTEM
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace std_fs = std::filesystem;
#endif

#if WEBCFACE_SYSTEM_PATH_WINDOWS
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <climits>
#else
#include <unistd.h>
// #include <linux/limits.h>
// manually define for cygwin
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
#endif

WEBCFACE_NS_BEGIN
namespace server {

// https://stackoverflow.com/questions/50889647/best-way-to-get-exe-folder-path
std_fs::path
getExeDir([[maybe_unused]] const std::shared_ptr<spdlog::logger> &logger) {
#if WEBCFACE_SYSTEM_PATH_WINDOWS
    // Windows specific
    wchar_t szPath[MAX_PATH];
    GetModuleFileNameW(NULL, szPath, MAX_PATH);
    return std_fs::path{szPath}.parent_path();
#elif __APPLE__
    char szPath[PATH_MAX];
    uint32_t bufsize = PATH_MAX;
    if (!_NSGetExecutablePath(szPath, &bufsize)) {
        return std_fs::path{szPath}
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
    return std_fs::path{szPath}.parent_path();
#endif
}
std::string getStaticDir(const std::shared_ptr<spdlog::logger> &logger) {
    auto exe_dir = getExeDir(logger);
    std::array<std_fs::path, 8> static_dirs = {
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
            if (std_fs::exists(dir / "index.html")) {
                logger->debug("{} found.", (dir / "index.html").string());
                return dir.string();
            } else {
                logger->debug("{} not found.", (dir / "index.html").string());
            }
        } catch (const std_fs::filesystem_error &e) {
            logger->warn("filesystem error on {}: {}",
                         (dir / "index.html").string(), e.what());
        }
    }
    logger->warn("Cannot find webui dist directory.");
    logger->debug("Falling back to <executable_dir>/dist");
    return (exe_dir / "dist").string();
}
std::string getTempDir(const std::shared_ptr<spdlog::logger> &logger) {
    try {
        return std_fs::temp_directory_path().string();
    } catch (const std_fs::filesystem_error &e) {
        logger->error("temp_directory_path error: {}", e.what());
        return "";
    }
}
} // namespace server
WEBCFACE_NS_END

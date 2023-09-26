#include "dir.h"
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#include <climits>
#else
#include <unistd.h>
#endif

namespace WebCFace {
namespace Server {

// https://stackoverflow.com/questions/50889647/best-way-to-get-exe-folder-path
std::filesystem::path getExeDir(const std::shared_ptr<spdlog::logger> &logger) {
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
    try {
        // .../build/
        auto dir = exe_dir.parent_path() / "webui" / "dist";
        if (std::filesystem::exists(dir / "index.html")) {
            return dir.string();
        }
    } catch (...) {
    }
    try {
        // .../bin/
        auto dir = exe_dir.parent_path() / "share" / "webcface" / "dist";
        if (std::filesystem::exists(dir / "index.html")) {
            return dir.string();
        }
    } catch (...) {
    }
    logger->critical("Cannot find static dir");
    return "";
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
} // namespace WebCFace
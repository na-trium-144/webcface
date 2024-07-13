#include "webcface/server/unix_path.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

WEBCFACE_NS_BEGIN
namespace message::Path {
std_fs::path unixSocketPath(int port) {
#ifdef _WIN32
    wchar_t *fpath;
    SHGetKnownFolderPath(FOLDERID_ProgramData, 0, nullptr, &fpath);
    return std_fs::path(fpath) / "webcface" / (std::to_string(port) + ".sock");
#else
    return "/tmp/webcface/" + std::to_string(port) + ".sock";
#endif
}

std_fs::path unixSocketPathWSLInterop(int port) {
    return "/mnt/c/ProgramData/webcface/" + std::to_string(port) + ".sock";
}

bool detectWSL1() {
    return std_fs::exists("/proc/sys/fs/binfmt_misc/WSLInterop") &&
           !std::getenv("WSL_INTEROP");
    // https://github.com/microsoft/WSL/issues/4555
}
bool detectWSL2() {
    return std_fs::exists("/proc/sys/fs/binfmt_misc/WSLInterop") &&
           std::getenv("WSL_INTEROP");
}
std::string wsl2Host() {
#ifdef _WIN32
    return "";
#else
    std::ifstream ifs("/proc/net/route");
    std::string ln;
    while (!ifs.eof()) {
        std::getline(ifs, ln);
        std::stringstream ln_ss(ln);
        std::string iface, dest, gateway;
        ln_ss >> iface >> dest >> gateway;
        if (dest == "00000000" && gateway.size() == 8) {
            std::string host_addr = "";
            for (int i = 3; i >= 0; i--) {
                host_addr += std::to_string(std::stoi(
                    gateway.substr(static_cast<std::size_t>(i) * 2, 2), nullptr,
                    16));
                if (i != 0) {
                    host_addr += '.';
                }
            }
            return host_addr;
        }
    }
    return "";
#endif
}

void initUnixSocket(const std_fs::path &path,
                    const std::shared_ptr<spdlog::logger> &logger) {
    try {
        std_fs::create_directories(path.parent_path());
    } catch (const std_fs::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
#ifdef _WIN32
    // std_fs does not work on socket file somehow on mingw
    DeleteFileW(path.wstring().c_str());
    auto dw = GetLastError();
    if (dw != 0 && dw != ERROR_FILE_NOT_FOUND) {
        char *lpMsgBuf = nullptr;
        FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                           FORMAT_MESSAGE_FROM_SYSTEM |
                           FORMAT_MESSAGE_IGNORE_INSERTS,
                       nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                       reinterpret_cast<LPSTR>(&lpMsgBuf), 0, nullptr);
        logger->warn("DeleteFile ({}) failed: {}", path.string(), lpMsgBuf);
        LocalFree(lpMsgBuf);
    }
#else
    try {
        std_fs::remove(path);
    } catch (const std_fs::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
#endif
}
void updateUnixSocketPerms(const std_fs::path &path,
                           const std::shared_ptr<spdlog::logger> &logger) {
    try {
        std_fs::permissions(path.parent_path(), std_fs::perms::all);
    } catch (const std_fs::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
    try {
        std_fs::permissions(path, std_fs::perms::all);
    } catch (const std_fs::filesystem_error &e) {
        logger->warn("{}", e.what());
    }
}

} // namespace message::Path
WEBCFACE_NS_END

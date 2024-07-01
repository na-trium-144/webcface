#pragma once
#include <functional>

namespace webcface {
namespace server_internal {
using wsConnPtr = void *;

using LoggerCallback =
    std::function<void(const char *, unsigned long long, int)>;
using OpenCallback = std::function<void(void *, const char *)>;
using CloseCallback = std::function<void(void *, const char *)>;
using MessageCallback =
    std::function<void(void *, const char *, unsigned long long)>;
using StartCallback = std::function<void()>;

class AppWrapper {
    void *app = nullptr;
    void *exception_str = nullptr;

    void setException(const char *what) noexcept;

  public:
    AppWrapper(const LoggerCallback &callback, const char *static_dir, int port,
               const char *unix_path, const OpenCallback &on_open,
               const CloseCallback &on_close, const MessageCallback &on_message,
               const StartCallback &on_start) noexcept;
    ~AppWrapper() noexcept;
    static void send(wsConnPtr conn, const char *msg,
                     unsigned long long size) noexcept;
    void stop() noexcept;
    void run() noexcept;
    const char *exception() noexcept;
};
} // namespace server_internal
} // namespace webcface

#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <cstdint>
#include <functional>
#include <string>

WEBCFACE_NS_BEGIN
namespace server {
using wsConnPtr = void *;

using LoggerCallback = std::function<void(const char *, std::size_t, int)>;
using OpenCallback = std::function<void(void *, const char *)>;
using CloseCallback = std::function<void(void *, const char *)>;
using MessageCallback = std::function<void(void *, const char *, std::size_t)>;
using StartCallback = std::function<void()>;

class AppWrapper {
    void *app = nullptr;
    std::string exception_str;

    void setException(const char *what) noexcept;

  public:
    AppWrapper(const LoggerCallback &callback, const char *static_dir,
               std::uint16_t port, const char *unix_path,
               const OpenCallback &on_open, const CloseCallback &on_close,
               const MessageCallback &on_message,
               const StartCallback &on_start) noexcept;
    ~AppWrapper() noexcept;
    static void send(wsConnPtr conn, const char *msg,
                     std::size_t size) noexcept;
    void stop() noexcept;
    void run() noexcept;
    const char *exception() noexcept;
};
} // namespace server
WEBCFACE_NS_END

#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>
#include <vector>

WEBCFACE_NS_BEGIN
namespace server {
class ServerStorage;
struct MemberData;

using wsConnPtr = void *;

void initVips();

class Server {
    std::atomic<bool> server_stop;
    std::mutex server_mtx;
    std::vector<void *> apps;
    std::vector<std::thread> apps_running;

    void pingThreadMain();

    void send(wsConnPtr conn, const std::string &msg);

  public:
    friend MemberData;
    std::condition_variable server_ping_wait;
    std::unique_ptr<ServerStorage> store;

  private:
    std::thread ping_thread; // storeよりも後ろ

  public:
    Server(std::uint16_t port, int level, int keep_log = 1000,
           spdlog::sink_ptr sink = nullptr,
           std::shared_ptr<spdlog::logger> logger = nullptr);
    ~Server();
    void join();
    void stop();

    Server(const Server &) = delete;
    Server &operator=(const Server &) = delete;
    Server(Server &&) = delete;
    Server &operator=(Server &&) = delete;
};
} // namespace server
WEBCFACE_NS_END

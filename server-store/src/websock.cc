#include "webcface/server/store.h"
#include "webcface/server/member_data.h"
#include "webcface/server/dir.h"
#include "webcface/server/ip.h"
#include "webcface/server/server.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/message/message.h"
#include "webcface/internal/unix_path.h"
#include <memory>
#include <thread>
#include "webcface/server/server_ws.h"
#include <spdlog/sinks/stdout_color_sinks.h>

WEBCFACE_NS_BEGIN
namespace server {

static const std::thread::id MAIN_THREAD_ID = std::this_thread::get_id();

static inline spdlog::level::level_enum convertLevel(int level) {
    switch (level) {
    case 4:
        return spdlog::level::critical;
    case 3:
        return spdlog::level::err;
    case 2:
        return spdlog::level::warn;
    case 1:
        return spdlog::level::info;
    case 0:
    default:
        return spdlog::level::debug;
    }
}
void Server::pingThreadMain() {
    std::unique_lock lock(server_mtx);
    while (!server_stop.load()) {
        // ping_interval経過するかserver_stop_condで起こされるまで待機
        server_ping_wait.wait_for(lock, MemberData::ping_interval);

        if (server_stop.load()) {
            return;
        }
        auto new_ping_status =
            std::make_shared<std::unordered_map<unsigned int, int>>();
        store->forEach([&](auto cd) {
            if (cd->last_ping_duration) {
                new_ping_status->emplace(
                    cd->member_id,
                    static_cast<int>(cd->last_ping_duration->count()));
            }
        });
        store->ping_status = new_ping_status;
        auto msg =
            message::packSingle(message::PingStatus{{}, store->ping_status});
        store->forEach([&](auto cd) {
            cd->logger->trace("ping");
            cd->sendPing();
            if (cd->ping_status_req) {
                cd->send(msg);
                cd->logger->trace("send ping_status");
            }
        });
    }
}
void Server::send(wsConnPtr conn, const std::string &msg) {
    if (!server_stop.load()) {
        AppWrapper::send(conn, msg.data(), msg.size());
    }
}
void Server::join() {
    static std::mutex join_m;
    std::lock_guard lock(join_m);
    for (auto &running_th : apps_running) {
        if (running_th.joinable()) {
            running_th.join();
        }
    }
}
Server::~Server() {
    {
        std::lock_guard lock(server_mtx);
        server_stop.store(true);
    }
    server_ping_wait.notify_one();
    for (auto &app : apps) {
        static_cast<AppWrapper *>(app)->stop();
    }
    ping_thread.join();
    this->join();
    store.reset();
    for (auto &app : apps) {
        delete static_cast<AppWrapper *>(app);
    }
}

Server::Server(std::uint16_t port, int level, int keep_log)
    : server_stop(false), apps(), apps_running(), server_ping_wait(),
      store(std::make_unique<ServerStorage>(this, keep_log)),
      ping_thread([this] { pingThreadMain(); }) {

    auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    auto logger = std::make_shared<spdlog::logger>("webcface_server", sink);
    logger->set_level(static_cast<spdlog::level::level_enum>(level));
    logger->info("WebCFace Server {}", WEBCFACE_VERSION);

    if (std::this_thread::get_id() != MAIN_THREAD_ID) {
        logger->warn("Initialization of webcface::Server::Server should be "
                     "called in the main thread (to initialize ImageMagick).");
    }
    initMagick();

    auto crow_logger = std::make_shared<spdlog::logger>("crow_server", sink);
    crow_logger->set_level(spdlog::level::trace);
    auto crow_logger_callback = [crow_logger](const char *data,
                                              std::size_t size, int level) {
        crow_logger->log(convertLevel(level), std::string(data, size));
    };

    auto static_dir = getStaticDir(logger);
    // auto temp_dir = getTempDir(logger);
    logger->debug("static dir = {}", static_dir);
    // logger->debug("temp dir = {}", temp_dir);

    store->hostname = getHostName(logger);
    logger->info("HostName is {}", store->hostname);

    auto unix_path = internal::unixSocketPath(port);
    auto wsl_path = internal::unixSocketPathWSLInterop(port);

    auto open_callback = [this, sink, level](void *conn, const char *ip) {
        std::lock_guard lock(server_mtx);
        store->newClient(conn, ip, sink,
                         static_cast<spdlog::level::level_enum>(level));
    };
    auto close_callback = [this](void *conn, const char * /*reason*/) {
        std::lock_guard lock(server_mtx);
        store->removeClient(conn);
    };
    auto message_callback = [this](void *conn, const char *data,
                                   std::size_t size) {
        std::lock_guard lock(server_mtx);
        auto cli = store->getClient(conn);
        if (cli) {
            cli->onRecv(std::string(data, size));
        }
    };

    auto *app_tcp = new AppWrapper(
        crow_logger_callback, static_dir.c_str(), port, nullptr, open_callback,
        close_callback, message_callback, [logger, port]() {
            for (const auto &addr : getIpAddresses(logger)) {
                logger->info("http://{}:{}/index.html", addr, port);
            }
        });
    apps.push_back(app_tcp);

    internal::initUnixSocket(unix_path, logger);
    auto *app_unix = new AppWrapper(
        crow_logger_callback, static_dir.c_str(), port,
        unix_path.string().c_str(), open_callback, close_callback,
        message_callback, [unix_path, logger]() {
            internal::updateUnixSocketPerms(unix_path, logger);
            logger->info("unix domain socket at {}", unix_path.string());
        });
    apps.push_back(app_unix);

    AppWrapper *app_wsl = nullptr;
    if (internal::detectWSL1()) {
        internal::initUnixSocket(wsl_path, logger);
        app_wsl = new AppWrapper(
            crow_logger_callback, static_dir.c_str(), port,
            wsl_path.string().c_str(), open_callback, close_callback,
            message_callback, [wsl_path, logger]() {
                internal::updateUnixSocketPerms(wsl_path, logger);
                logger->info("win32 socket at {}", wsl_path.string());
            });
        apps.push_back(app_wsl);
    }

    for (auto &app_v : apps) {
        auto app = static_cast<AppWrapper *>(app_v);

        apps_running.emplace_back([app, logger] {
            app->run();
            if (app->exception()) {
                logger->error("{}", app->exception());
            }
        });
    }
}
} // namespace server
WEBCFACE_NS_END

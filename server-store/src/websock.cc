#include "webcface/server/store.h"
#include "webcface/server/member_data.h"
#include "webcface/server/dir.h"
#include "webcface/server/ip.h"
#include <webcface/server/server.h>
#include <webcface/common/def.h>
#include "webcface/message/message.h"
#include "webcface/server/unix_path.h"
#include <memory>
#include <thread>
#include "webcface/server/internal/server_ws.h"

WEBCFACE_NS_BEGIN
namespace Server {

static spdlog::level::level_enum convertLevel(int level) {
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
                new_ping_status->emplace(cd->member_id,
                                         cd->last_ping_duration->count());
            }
        });
        store->ping_status = new_ping_status;
        auto msg =
            Message::packSingle(Message::PingStatus{{}, store->ping_status});
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
        webcfaceServerInternal::AppWrapper::send(conn, msg.data(), msg.size());
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
        static_cast<webcfaceServerInternal::AppWrapper *>(app)->stop();
    }
    ping_thread.join();
    this->join();
    store.reset();
    for (auto &app : apps) {
        delete static_cast<webcfaceServerInternal::AppWrapper *>(app);
    }
}
Server::Server(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level, int keep_log)
    : server_stop(false), apps(), apps_running(), server_ping_wait(),
      store(std::make_unique<ServerStorage>(this, keep_log)),
      ping_thread([this] { pingThreadMain(); }) {
    auto logger = std::make_shared<spdlog::logger>("webcface_server", sink);
    logger->set_level(spdlog::level::trace);
    logger->info("WebCFace Server {}", WEBCFACE_VERSION);

    auto crow_logger = std::make_shared<spdlog::logger>("crow_server", sink);
    crow_logger->set_level(spdlog::level::trace);
    auto crow_logger_callback =
        [crow_logger](const char *data, unsigned long long size, int level) {
            crow_logger->log(convertLevel(level), std::string(data, size));
        };

    auto static_dir = getStaticDir(logger);
    // auto temp_dir = getTempDir(logger);
    logger->debug("static dir = {}", static_dir);
    // logger->debug("temp dir = {}", temp_dir);

    auto unix_path = Message::Path::unixSocketPath(port);
    auto wsl_path = Message::Path::unixSocketPathWSLInterop(port);

    auto open_callback = [this, sink, level](void *conn, const char *ip) {
        std::lock_guard lock(server_mtx);
        store->newClient(conn, ip, sink, level);
    };
    auto close_callback = [this](void *conn, const char * /*reason*/) {
        std::lock_guard lock(server_mtx);
        store->removeClient(conn);
    };
    auto message_callback = [this](void *conn, const char *data,
                                   unsigned long long size) {
        std::lock_guard lock(server_mtx);
        auto cli = store->getClient(conn);
        if (cli) {
            cli->onRecv(std::string(data, size));
        }
    };

    auto *app_tcp = new webcfaceServerInternal::AppWrapper(
        crow_logger_callback, static_dir.c_str(), port, nullptr, open_callback,
        close_callback, message_callback, [logger, port]() {
            for (const auto &addr : getIpAddresses(logger)) {
                logger->info("http://{}:{}/index.html", addr, port);
            }
        });
    apps.push_back(app_tcp);

    Message::Path::initUnixSocket(unix_path, logger);
    auto *app_unix = new webcfaceServerInternal::AppWrapper(
        crow_logger_callback, static_dir.c_str(), port,
        unix_path.string().c_str(), open_callback, close_callback,
        message_callback, [unix_path, logger]() {
            Message::Path::updateUnixSocketPerms(unix_path, logger);
            logger->info("unix domain socket at {}", unix_path.string());
        });
    apps.push_back(app_unix);

    webcfaceServerInternal::AppWrapper *app_wsl = nullptr;
    if (Message::Path::detectWSL1()) {
        Message::Path::initUnixSocket(wsl_path, logger);
        app_wsl = new webcfaceServerInternal::AppWrapper(
            crow_logger_callback, static_dir.c_str(), port,
            wsl_path.string().c_str(), open_callback, close_callback,
            message_callback, [wsl_path, logger]() {
                Message::Path::updateUnixSocketPerms(wsl_path, logger);
                logger->info("win32 socket at {}", wsl_path.string());
            });
        apps.push_back(app_wsl);
    }

    for (auto &app_v : apps) {
        auto app = static_cast<webcfaceServerInternal::AppWrapper *>(app_v);

        apps_running.emplace_back([app, logger] {
            app->run();
            if (app->exception()) {
                logger->error("{}", app->exception());
            }
        });
    }
}
} // namespace Server
WEBCFACE_NS_END

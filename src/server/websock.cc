#include "store.h"
#include "member_data.h"
#include "dir.h"
#include "ip.h"
#include <webcface/server.h>
#include <webcface/common/def.h>
#include "../message/message.h"
#include "../message/unix_path.h"
#include <memory>
#include <thread>
#include <filesystem>

WEBCFACE_NS_BEGIN
namespace Server {
static std::string static_dir;
}
WEBCFACE_NS_END

#define CROW_STATIC_DIRECTORY webcface::Server::static_dir
#define CROW_STATIC_ENDPOINT "/<path>"
#include <crow.h>

#include "custom_logger.h"

WEBCFACE_NS_BEGIN
namespace Server {

static std::unique_ptr<CustomLogger> crow_custom_logger;

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
        static_cast<crow::websocket::connection *>(conn)->send_binary(msg);
    }
}
void Server::join() {
    for (auto &running_f : apps_running) {
        running_f.wait();
    }
}
Server::~Server() {
    {
        std::lock_guard lock(server_mtx);
        server_stop.store(true);
    }
    server_ping_wait.notify_one();
    for (auto &app : apps) {
        static_cast<crow::SimpleApp *>(app)->stop();
    }
    ping_thread.join();
    for (auto &running_f : apps_running) {
        running_f.wait();
    }
    store.reset();
}
Server::Server(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level, int keep_log)
    : server_stop(false), apps(), apps_running(), server_ping_wait(),
      store(std::make_unique<ServerStorage>(this, keep_log)),
      ping_thread([this] { pingThreadMain(); }) {
    auto logger = std::make_shared<spdlog::logger>("webcface_server", sink);
    logger->set_level(spdlog::level::trace);
    logger->info("WebCFace Server {}", WEBCFACE_VERSION);

    if (!crow_custom_logger) {
        auto crow_logger =
            std::make_shared<spdlog::logger>("crow_server", sink);
        crow_logger->set_level(spdlog::level::trace);
        crow_custom_logger = std::make_unique<CustomLogger>(crow_logger);
        crow::logger::setHandler(crow_custom_logger.get());
    }

    static_dir = getStaticDir(logger);
    auto temp_dir = getTempDir(logger);
    logger->debug("static dir = {}", static_dir);
    logger->debug("temp dir = {}", temp_dir);

    auto unix_path = Message::Path::unixSocketPath(port);
    auto wsl_path = Message::Path::unixSocketPathWSLInterop(port);

    crow::SimpleApp *app_tcp = new crow::SimpleApp();
    app_tcp->port(port);
    apps.push_back(app_tcp);

    crow::SimpleApp *app_unix = new crow::SimpleApp();
    Message::Path::initUnixSocket(unix_path, logger);
    app_unix->unix_path(unix_path.string());
    apps.push_back(app_unix);

    crow::SimpleApp *app_wsl = nullptr;
    if (Message::Path::detectWSL1()) {
        app_wsl = new crow::SimpleApp();
        Message::Path::initUnixSocket(wsl_path, logger);
        app_wsl->unix_path(wsl_path.string());
        apps.push_back(app_wsl);
    }

    for (auto &app_v : apps) {
        auto app = static_cast<crow::SimpleApp *>(app_v);
        app->loglevel(crow::LogLevel::Warning);

        /*
        / にアクセスしたときindex.htmlへリダイレクトさせようとしたが、
        windowsでなんかうまくいかなかったので諦めた

        auto &route = CROW_ROUTE((*app), "/");
        route([](crow::response &res) {
            res.redirect("index.html");
            res.end();
        });
        route.websocket<std::remove_reference<decltype(*app)>::type>(app.get())
        */
        CROW_WEBSOCKET_ROUTE((*app), "/")
            .onopen([this, sink, level](crow::websocket::connection &conn) {
                std::lock_guard lock(server_mtx);
                store->newClient(&conn, conn.get_remote_ip(), sink, level);
            })
            .onclose([this](crow::websocket::connection &conn,
                            const std::string & /*reason*/) {
                std::lock_guard lock(server_mtx);
                store->removeClient(&conn);
            })
            .onmessage([this](crow::websocket::connection &conn,
                              const std::string &data, bool /*is_binary*/) {
                std::lock_guard lock(server_mtx);
                auto cli = store->getClient(&conn);
                if (cli) {
                    cli->onRecv(data);
                }
            });

        auto f = app->run_async().share();
        apps_running.push_back(f);
        std::thread([f, logger] {
            try {
                f.get();
            } catch (const std::exception &e) {
                logger->error("{}", e.what());
            }
        }).detach();
    }

    std::thread([app_tcp, logger, port] {
        app_tcp->wait_for_server_start();
        for (const auto &addr : getIpAddresses(logger)) {
            logger->info("http://{}:{}/index.html", addr, port);
        }
    }).detach();
    std::thread([app_unix, unix_path, logger] {
        app_unix->wait_for_server_start();
        Message::Path::updateUnixSocketPerms(unix_path, logger);
        logger->info("unix domain socket at {}", unix_path.string());
    }).detach();
    if (app_wsl) {
        std::thread([app_wsl, wsl_path, logger] {
            app_wsl->wait_for_server_start();
            Message::Path::updateUnixSocketPerms(wsl_path, logger);
            logger->info("win32 socket at {}", wsl_path.string());
        }).detach();
    }
}
} // namespace Server
WEBCFACE_NS_END

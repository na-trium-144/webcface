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
#ifdef _WIN32
#include <fileapi.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif

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
        running_f.get();
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
    store.reset();
    ping_thread.join();
    for (auto &running_f : apps_running) {
        running_f.get();
    }
}
Server::Server(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level, int keep_log)
    : server_stop(false), apps(), apps_running(),
      ping_thread([this] { pingThreadMain(); }), server_ping_wait(),
      store(std::make_unique<ServerStorage>(this, keep_log)) {
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

    auto unix_path = Message::unixSocketPath(port);
    auto wsl_path = Message::unixSocketPathWSLInterop(port);

    std::thread([port, unix_path, wsl_path, logger] {
        for (const auto &addr : getIpAddresses(logger)) {
            logger->info("http://{}:{}/index.html", addr, port);
        }
        logger->info("unix domain socket at {}", unix_path.string());
        if (wsl_path) {
            logger->info("win32 socket at {}", wsl_path->string());
        }
    }).detach();

    crow::SimpleApp *app_tcp = new crow::SimpleApp();
    app_tcp->port(port);
    apps.push_back(app_tcp);

    crow::SimpleApp *app_unix = new crow::SimpleApp();
#ifdef _WIN32
    CreateDirectoryW(unix_path.parent_path().c_str(), nullptr);
    DeleteFileW(unix_path.c_str());
    app_unix->unix_path(unix_path);
#else
    mkdir(unix_path.parent_path().c_str(), 0777);
    unlink(unix_path.c_str());
    app_unix->unix_path(unix_path);
    std::thread([app_unix, unix_path] {
        app_unix->wait_for_server_start();
        chmod(unix_path.c_str(), 0666);
    }).detach();
#endif
    apps.push_back(app_unix);

    crow::SimpleApp *app_wsl;
    if (wsl_path) {
        app_wsl = new crow::SimpleApp();
        mkdir(wsl_path->parent_path().c_str(), 0777);
        unlink(wsl_path->c_str());
        app_wsl->unix_path(*wsl_path);
        std::thread([app_wsl, wsl_path] {
            app_wsl->wait_for_server_start();
            chmod(wsl_path->c_str(), 0666);
        }).detach();
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
            .onopen([&](crow::websocket::connection &conn) {
                std::lock_guard lock(server_mtx);
                store->newClient(&conn, conn.get_remote_ip(), sink, level);
            })
            .onclose([&](crow::websocket::connection &conn,
                         const std::string & /*reason*/) {
                std::lock_guard lock(server_mtx);
                store->removeClient(&conn);
            })
            .onmessage([&](crow::websocket::connection &conn,
                           const std::string &data, bool /*is_binary*/) {
                std::lock_guard lock(server_mtx);
                auto cli = store->getClient(&conn);
                if (cli) {
                    cli->onRecv(data);
                }
            });

        apps_running.push_back(app->run_async().share());
    }
}
} // namespace Server
WEBCFACE_NS_END

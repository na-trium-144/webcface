#include "websock.h"
#include "store.h"
#include "s_client_data.h"
#include "dir.h"
#include "ip.h"
#include <webcface/common/def.h>
#include <webcface/common/unix_path.h>
#include "../message/message.h"
#include <memory>
#include <thread>
#include <filesystem>
#include <unistd.h>
#include <sys/stat.h>

WEBCFACE_NS_BEGIN
namespace Server {
std::string static_dir;
}
WEBCFACE_NS_END
#define CROW_STATIC_DIRECTORY webcface::Server::static_dir
#define CROW_STATIC_ENDPOINT "/<path>"
#include <crow.h>

#include "custom_logger.h"

WEBCFACE_NS_BEGIN
namespace Server {

std::array<std::unique_ptr<crow::SimpleApp>, 2> apps;
std::unique_ptr<std::thread> ping_thread;
std::unique_ptr<CustomLogger> crow_custom_logger;
std::atomic<bool> server_stop;

void pingThreadMain() {
    std::unique_lock lock(server_mtx);
    while (!server_stop.load()) {
        // ping_interval経過するかserver_stop_condで起こされるまで待機
        server_ping_wait.wait_for(lock, ClientData::ping_interval);

        if (server_stop.load()) {
            return;
        }
        auto new_ping_status =
            std::make_shared<std::unordered_map<unsigned int, int>>();
        store.forEach([&](auto cd) {
            if (cd->last_ping_duration) {
                new_ping_status->emplace(cd->member_id,
                                         cd->last_ping_duration->count());
            }
        });
        ping_status = new_ping_status;
        auto msg = Message::packSingle(Message::PingStatus{{}, ping_status});
        store.forEach([&](auto cd) {
            cd->logger->trace("ping");
            cd->sendPing();
            if (cd->ping_status_req) {
                cd->send(msg);
                cd->logger->trace("send ping_status");
            }
        });
    }
}
void serverSend(void *conn, const std::string &msg) {
    if (!server_stop.load()) {
        reinterpret_cast<crow::websocket::connection *>(conn)->send_binary(msg);
    }
}
void serverStop() {
    if (!server_stop.load()) {
        {
            std::lock_guard lock(server_mtx);
            server_stop.store(true);
        }
        server_ping_wait.notify_one();
        for (auto &app : apps) {
            if (app) {
                app->stop();
            }
        }
        ping_thread->join();
    }
}
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level) {
    auto logger = std::make_shared<spdlog::logger>("webcface_server", sink);
    logger->set_level(spdlog::level::trace);

    logger->info("WebCFace Server {}", WEBCFACE_VERSION);
    std::thread([logger, port] {
        for (const auto &addr : getIpAddresses(logger)) {
            logger->info("http://{}:{}/index.html", addr, port);
        }
        logger->info("unix domain socket at {}",
                     Common::unixSocketPath(port).native());
    }).detach();

    auto crow_logger = std::make_shared<spdlog::logger>("crow_server", sink);
    crow_logger->set_level(spdlog::level::trace);
    crow_custom_logger = std::make_unique<CustomLogger>(crow_logger);
    crow::logger::setHandler(crow_custom_logger.get());

    server_stop.store(false);
    ping_thread = std::make_unique<std::thread>(pingThreadMain);

    static_dir = getStaticDir(logger);
    auto temp_dir = getTempDir(logger);
    logger->debug("static dir = {}", static_dir);
    logger->debug("temp dir = {}", temp_dir);

    for (auto &app : apps) {
        app = std::make_unique<crow::SimpleApp>();
        // app->loglevel(crow::LogLevel::Warning);

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
                store.newClient(&conn, conn.get_remote_ip(), sink, level);
            })
            .onclose([&](crow::websocket::connection &conn,
                         const std::string & /*reason*/) {
                std::lock_guard lock(server_mtx);
                store.removeClient(&conn);
            })
            .onmessage([&](crow::websocket::connection &conn,
                           const std::string &data, bool /*is_binary*/) {
                std::lock_guard lock(server_mtx);
                auto cli = store.getClient(&conn);
                if (cli) {
                    cli->onRecv(data);
                }
            });
    }

    std::array<std::future<void>, 2> apps_f;

    auto unix_path = Common::unixSocketPath(port);
    mkdir(unix_path.parent_path().c_str(), 0777);
    unlink(unix_path.c_str());
    apps_f[0] = apps[0]->unix_path(unix_path.native()).run_async();
    apps[0]->wait_for_server_start();
    chmod(unix_path.c_str(), 0666);

    apps_f[1] = apps[1]->port(port).run_async();

    for (auto &f : apps_f) {
        f.get();
    }

    serverStop();
}
} // namespace Server
WEBCFACE_NS_END

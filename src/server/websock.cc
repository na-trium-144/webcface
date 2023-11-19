#include "websock.h"
#include "store.h"
#include "s_client_data.h"
#include "dir.h"
#include <webcface/common/def.h>
#include "../message/message.h"
#include <memory>
#include <thread>

namespace WebCFace::Server {
std::string static_dir;
}
#define CROW_STATIC_DIRECTORY WebCFace::Server::static_dir
#define CROW_STATIC_ENDPOINT "/<path>"
#include <crow.h>

namespace WebCFace::Server {

class CustomLogger : public crow::ILogHandler {
    std::shared_ptr<spdlog::logger> logger;

  public:
    CustomLogger(const spdlog::sink_ptr &sink) {
        logger = std::make_shared<spdlog::logger>("crow_server", sink);
        logger->set_level(spdlog::level::trace);
    }
    void log(std::string message, crow::LogLevel level) {
        logger->log(convertLevel(level), message);
    }
    spdlog::level::level_enum convertLevel(crow::LogLevel level) {
        switch (level) {
        case crow::LogLevel::CRITICAL:
            return spdlog::level::critical;
        case crow::LogLevel::ERROR:
            return spdlog::level::err;
        case crow::LogLevel::WARNING:
            return spdlog::level::warn;
        case crow::LogLevel::INFO:
            return spdlog::level::info;
        case crow::LogLevel::DEBUG:
        default:
            return spdlog::level::debug;
        }
    }
};

std::unique_ptr<crow::SimpleApp> app;
std::unique_ptr<std::thread> ping_thread;
std::unique_ptr<CustomLogger> crow_logger;

void pingThreadMain() {
    std::unique_lock lock(server_mtx);
    while (!server_stop) {
        // ping_interval経過するかserver_stop_condで起こされるまで待機
        server_ping_wait.wait_for(lock, ClientData::ping_interval);

        if (server_stop) {
            return;
        }
        auto new_ping_status =
            std::make_shared<std::unordered_map<unsigned int, int>>();
        store.forEach([&](auto &cd) {
            if (cd.last_ping_duration) {
                new_ping_status->emplace(cd.member_id,
                                         cd.last_ping_duration->count());
            }
        });
        ping_status = new_ping_status;
        auto msg = Message::packSingle(Message::PingStatus{{}, ping_status});
        store.forEach([&](auto &cd) {
            cd.logger->trace("ping");
            cd.sendPing();
            if (cd.ping_status_req) {
                cd.send(msg);
                cd.logger->trace("send ping_status");
            }
        });
    }
}
void serverSend(void *conn, const std::string &msg) {
    reinterpret_cast<crow::websocket::connection *>(conn)->send_binary(msg);
}
void serverStop() {
    {
        std::lock_guard lock(server_mtx);
        server_stop = true;
    }
    server_ping_wait.notify_one();
    app->stop();
    ping_thread->join();
}
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level) {
    auto logger = std::make_shared<spdlog::logger>("webcface_server", sink);
    logger->set_level(spdlog::level::trace);

    logger->info("WebCFace Server {}", WEBCFACE_VERSION);
    logger->info("http://localhost:{}", port);

    crow_logger = std::make_unique<CustomLogger>(sink);
    crow::logger::setHandler(crow_logger.get());

    server_stop = false;
    ping_thread = std::make_unique<std::thread>(pingThreadMain);

    static_dir = getStaticDir(logger);
    auto temp_dir = getTempDir(logger);
    logger->debug("static dir = {}", static_dir);
    logger->debug("temp dir = {}", temp_dir);

    app = std::make_unique<crow::SimpleApp>();

    auto &route = CROW_ROUTE((*app), "/");
    route([](crow::response &res) {
        res.redirect("index.html");
        res.end();
    });
    // CROW_WEBSOCKET_ROUTE((*app), "/")
    route.websocket<std::remove_reference<decltype(*app)>::type>(app.get())
        .onopen([&](crow::websocket::connection &conn) {
            std::lock_guard lock(server_mtx);
            store.newClient(&conn, conn.get_remote_ip(), sink, level);
        })
        .onclose(
            [&](crow::websocket::connection &conn, const std::string &reason) {
                std::lock_guard lock(server_mtx);
                auto cli = store.getClient(&conn);
                if (cli) {
                    cli->con = nullptr;
                }
                store.removeClient(&conn);
            })
        .onmessage([&](crow::websocket::connection &conn,
                       const std::string &data, bool is_binary) {
            std::lock_guard lock(server_mtx);
            auto cli = store.getClient(&conn);
            if (cli) {
                cli->onRecv(data);
            }
        });

    app->port(port).run();
    serverStop();
}
} // namespace WebCFace::Server

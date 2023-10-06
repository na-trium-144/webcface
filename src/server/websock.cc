#include "websock.h"
#include "store.h"
#include "s_client_data.h"
#include "dir.h"
#include <webcface/common/def.h>
#include "../message/message.h"
#include <cinatra.hpp>
#include <memory>
#include <thread>
namespace WebCFace::Server {
std::shared_ptr<cinatra::http_server> server;
std::shared_ptr<std::thread> ping_thread;

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
void serverStop() {
    {
        std::lock_guard lock(server_mtx);
        server_stop = true;
    }
    server_ping_wait.notify_one();
    server->stop();
    ping_thread->join();
}
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level) {
    auto logger = std::make_shared<spdlog::logger>("webcface_server", sink);
    logger->set_level(spdlog::level::trace);

    logger->info("WebCFace Server {}", WEBCFACE_VERSION);
    logger->info("http://localhost:{}", port);

    using namespace cinatra;
    server_stop = false;
    ping_thread = std::make_shared<std::thread>(pingThreadMain);

    server = std::make_shared<http_server>(1);
    auto static_dir = getStaticDir(logger);
    auto temp_dir = getTempDir(logger);
    server->set_static_dir(static_dir);
    server->set_upload_dir(temp_dir);
    logger->debug("static dir = {}", static_dir);
    logger->debug("temp dir = {}", temp_dir);

    server->listen("0.0.0.0", std::to_string(port));

    // web socket
    server->set_http_handler<GET, POST>(
        "/", [sink, level](request &req, response &res) {
            if (req.get_content_type() == content_type::websocket) {

                req.on(ws_open, [sink, level](request &req) {
                    std::lock_guard lock(server_mtx);
                    auto connPtr = std::static_pointer_cast<void>(
                        req.get_conn<cinatra::NonSSL>());
                    store.newClient(connPtr, sink, level);
                });

                req.on(ws_message, [](request &req) {
                    std::lock_guard lock(server_mtx);
                    auto part_data = req.get_part_data();
                    // echo
                    std::string str =
                        std::string(part_data.data(), part_data.length());
                    // req.get_conn<cinatra::NonSSL>()->send_ws_string(std::move(str));
                    // std::cout << part_data.data() << std::endl;

                    auto connPtr = std::static_pointer_cast<void>(
                        req.get_conn<cinatra::NonSSL>());
                    auto cli = store.getClient(connPtr);
                    if (cli) {
                        cli->onRecv(str);
                    }

                    // なんか送り返さないと受信できなくなるっぽい? 謎
                    req.get_conn<cinatra::NonSSL>()->send_ws_binary("");
                });

                req.on(ws_error, [](request &req) {
                    std::lock_guard lock(server_mtx);
                    auto connPtr = std::static_pointer_cast<void>(
                        req.get_conn<cinatra::NonSSL>());
                    store.removeClient(connPtr);
                });

                req.on(ws_close, [](request &req) {
                    std::lock_guard lock(server_mtx);
                    auto connPtr = std::static_pointer_cast<void>(
                        req.get_conn<cinatra::NonSSL>());
                    store.removeClient(connPtr);
                });

            } else {
                // ブラウザからアクセスされたらindex.htmlへリダイレクトする
                res.redirect("index.html", true);
            }
        });

    server->run();
}
} // namespace WebCFace::Server

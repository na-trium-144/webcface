#include "websock.h"
#include "store.h"
#include "s_client_data.h"
#include "../message/message.h"
#include <cinatra.hpp>
#include <memory>
#include <thread>

namespace WebCFace::Server {
std::shared_ptr<cinatra::http_server> server;
std::shared_ptr<std::thread> ping_thread;

void pingThreadMain() {
    while (true) {
        std::unique_lock lock(server_mtx);
        server_stop_cond.wait_for(lock, ClientData::ping_interval,
                                  [] { return server_stop });
        if (server_stop) {
            return;
        }
        auto ping_status =
            std::make_shared<std::unordered_map<unsigned int, int>>();
        store.forEach([&](auto &cd) {
            if (cd.last_ping_duration) {
                ping_status->emplace(cd.member_id,
                                     cd.last_ping_duration->count());
            }
        });
        auto msg = Message::packSingle(Message::PingStatus{{}, ping_status});
        store.forEach([](auto &cd) {
            cd.sendPing();
            if (cd.ping_status_req) {
                cd.send(msg);
            }
        });
    }
}
void serverStop() {
    {
        std::lock_guard lock(server_mtx);
        server_stop = true;
    }
    server_stop_cond.notify_one();
    server->stop();
    ping_thread.join();
}
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level) {
    using namespace cinatra;
    server_stop = false;
    ping_thread = std::make_shared<std::thread>(pingThreadMain);

    server = std::make_shared<http_server>(1);
    server->listen("0.0.0.0", std::to_string(port));

    // web socket
    server->set_http_handler<GET, POST>("/", [sink, level](request &req,
                                                           response &res) {
        assert(req.get_content_type() == content_type::websocket);

        req.on(ws_open, [sink, level](request &req) {
            std::lock_guard lock(server_mtx);
            auto connPtr =
                std::static_pointer_cast<void>(req.get_conn<cinatra::NonSSL>());
            store.newClient(connPtr, sink, level);
        });

        req.on(ws_message, [](request &req) {
            std::lock_guard lock(server_mtx);
            auto part_data = req.get_part_data();
            // echo
            std::string str = std::string(part_data.data(), part_data.length());
            // req.get_conn<cinatra::NonSSL>()->send_ws_string(std::move(str));
            // std::cout << part_data.data() << std::endl;

            auto connPtr =
                std::static_pointer_cast<void>(req.get_conn<cinatra::NonSSL>());
            auto cli = store.getClient(connPtr);
            if (cli) {
                cli->onRecv(str);
            }

            // なんか送り返さないと受信できなくなるっぽい? 謎
            req.get_conn<cinatra::NonSSL>()->send_ws_binary("");
        });

        req.on(ws_error, [](request &req) {
            std::lock_guard lock(server_mtx);
            auto connPtr =
                std::static_pointer_cast<void>(req.get_conn<cinatra::NonSSL>());
            store.removeClient(connPtr);
        });

        req.on(ws_close, [](request &req) {
            std::lock_guard lock(server_mtx);
            auto connPtr =
                std::static_pointer_cast<void>(req.get_conn<cinatra::NonSSL>());
            store.removeClient(connPtr);
        });
    });

    server->run();
}
} // namespace WebCFace::Server

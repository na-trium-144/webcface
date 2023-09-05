#include "websock.h"
#include "../server/store.h"
#include <cinatra.hpp>
#include <string>
#include <iostream>
#include <memory>

namespace WebCFace::Server {
void serverRun(int port, const spdlog::sink_ptr &sink,
               spdlog::level::level_enum level) {
    using namespace cinatra;

    http_server server(1);
    server.listen("0.0.0.0", std::to_string(port));

    // web socket
    server.set_http_handler<GET, POST>("/", [sink, level](request &req,
                                                          response &res) {
        assert(req.get_content_type() == content_type::websocket);

        req.on(ws_open, [sink, level](request &req) {
            auto connPtr =
                std::static_pointer_cast<void>(req.get_conn<cinatra::NonSSL>());
            store.newClient(connPtr, sink, level);
        });

        req.on(ws_message, [](request &req) {
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
        });

        req.on(ws_error, [](request &req) {
            auto connPtr =
                std::static_pointer_cast<void>(req.get_conn<cinatra::NonSSL>());
            store.removeClient(connPtr);
        });

        req.on(ws_close, [](request &req) {
            auto connPtr =
                std::static_pointer_cast<void>(req.get_conn<cinatra::NonSSL>());
            store.removeClient(connPtr);
        });
    });

    server.run();
}
} // namespace WebCFace::Server

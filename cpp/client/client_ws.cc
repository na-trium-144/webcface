#include <webcface/client.h>
#include <cinatra.hpp>
#include <string>
#include <chrono>
namespace WebCFace {

void Client::messageThreadMain() {
    using namespace cinatra;
    while (!this->closing.load()) {
        coro_http_client client;
        this->connected_.store(false);
        this->data->logger_internal->trace("start connecting");
        bool ok = async_simple::coro::syncAwait(client.async_ws_connect(
            "ws://" + host + ":" + std::to_string(port)));
        this->data->logger_internal->trace("finished connecting, status={}",
                                           ok);
        if (ok) {
            this->connected_.store(true);
            this->data->logger_internal->debug("connected");
            client.on_ws_msg([&](resp_data data) {
                if (data.net_err) {
                    this->data->logger_internal->error("ws_msg net error {}",
                                                       data.net_err.message());
                    return;
                }
                this->data->logger_internal->trace("message received");
                this->onRecv(std::string(data.resp_body));
            });
            client.on_ws_close([&](auto &&) {
                this->data->logger_internal->debug("connection closed");
                this->connected_.store(false);
            });

            while (!this->closing.load() && this->connected_.load()) {
                auto msg = this->data->message_queue.pop(
                    std::chrono::milliseconds(10));
                if (msg) {
                    // this->send(*msg);
                    this->data->logger_internal->trace("sending message");
                    async_simple::coro::syncAwait(client.async_send_ws(
                        std::string(&(*msg)[0], msg->size()), true,
                        opcode::binary));
                }
            }

            async_simple::coro::syncAwait(client.async_send_ws_close());
            this->data->logger_internal->trace("closing");
        }
    }
}

bool Client::connected() const {
    // return ws && ws->getConnection() && ws->getConnection()->connected();
    return connected_.load();
}
void Client::reconnect() {}

void Client::send(const std::vector<char> &m) {
    // if (connected()) {
    //     ws->getConnection()->send(&m[0], m.size(),
    //                               drogon::WebSocketMessageType::Binary);
    // }
    data->message_queue.push(m);
}


} // namespace WebCFace
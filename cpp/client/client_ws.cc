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

        // thisセグフォの可能性を減らすためコピーしておく(どういうとき起きるのかわかってない)
        auto logger_internal = this->data->logger_internal;
        client.on_ws_msg([this, logger_internal](resp_data data) {
            if (data.net_err) {
                // どういう条件で起こるのかわかってない
                logger_internal->error("recv error {}",
                                                   data.net_err.message());
                return;
            }
            logger_internal->trace("message received");
            this->onRecv(std::string(data.resp_body));
            logger_internal->trace("message recv done");
        });
        client.on_ws_close([this, logger_internal](auto &&) {
            logger_internal->debug("connection closed");
            this->connected_.store(false);
        });

        this->data->logger_internal->trace("start connecting");
        bool ok = async_simple::coro::syncAwait(client.async_ws_connect(
            "ws://" + host + ":" + std::to_string(port)));
        this->data->logger_internal->trace("finished connecting, status={}",
                                           ok);
        if (ok) {
            this->sync_init.store(false);
            this->connected_.store(true);
            this->data->logger_internal->debug("connected");

            while (!this->closing.load() && this->connected_.load()) {
                auto msg = this->data->message_queue.pop(
                    std::chrono::milliseconds(10));
                if (msg) {
                    // this->send(*msg);
                    this->data->logger_internal->trace("sending message");
                    auto data = async_simple::coro::syncAwait(
                        client.async_send_ws(*msg, true, opcode::binary));
                    if (data.net_err) {
                        this->data->logger_internal->debug(
                            "send error {}", data.net_err.message());
                        this->connected_.store(false);
                    }
                }
            }

            async_simple::coro::syncAwait(client.async_send_ws_close());
            this->data->logger_internal->trace("closing");
        }
    }
}

bool Client::connected() const { return connected_.load(); }

} // namespace WebCFace
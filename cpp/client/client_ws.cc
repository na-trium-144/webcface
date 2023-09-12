#include <webcface/client.h>
#include <cinatra.hpp>
#include <string>
#include <chrono>
namespace WebCFace {

void Client::messageThreadMain(std::shared_ptr<ClientData> data,
                               std::string host, int port) {
    using namespace cinatra;
    while (!data->closing.load()) {
        coro_http_client client;
        data->connected_.store(false);

        // thisセグフォの可能性を減らすためコピーしておく(どういうとき起きるのかわかってない)
        auto logger_internal = data->logger_internal;
        if (!data) {
            std::cout << "data is nullptr!! (messageThreadMain)" << std::endl;
        }
        client.on_ws_msg([data](resp_data rdata) {
            if (data) {
                if (rdata.net_err) {
                    // どういう条件で起こるのかわかってない
                    data->logger_internal->error("recv error {}",
                                                 rdata.net_err.message());
                    return;
                }
                data->logger_internal->trace("message received");
                data->recv_queue.push(std::string(rdata.resp_body));
                data->logger_internal->trace("message recv done");
            } else {
                std::cout << "data is nullptr!! (on_ws_msg)" << std::endl;
            }
        });
        client.on_ws_close([data](auto &&) {
            if (data) {
                data->logger_internal->debug("connection closed");
                data->connected_.store(false);
            } else {
                std::cout << "data is nullptr!! (on_ws_close)" << std::endl;
            }
        });

        data->logger_internal->trace("start connecting");
        bool ok = async_simple::coro::syncAwait(client.async_ws_connect(
            "ws://" + host + ":" + std::to_string(port)));
        data->logger_internal->trace("finished connecting, status={}", ok);
        if (ok) {
            data->sync_init.store(false);
            data->connected_.store(true);
            data->logger_internal->debug("connected");

            while (!data->closing.load() && data->connected_.load()) {
                auto msg =
                    data->message_queue.pop(std::chrono::milliseconds(10));
                if (msg) {
                    // this->send(*msg);
                    data->logger_internal->trace("sending message");
                    auto rdata = async_simple::coro::syncAwait(
                        client.async_send_ws(*msg, true, opcode::binary));
                    if (rdata.net_err) {
                        data->logger_internal->debug("send error {}",
                                                     rdata.net_err.message());
                        data->connected_.store(false);
                    }
                }
            }

            async_simple::coro::syncAwait(client.async_send_ws_close());
            data->logger_internal->trace("closing");
        }
    }
}

} // namespace WebCFace
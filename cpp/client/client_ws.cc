#include <webcface/client.h>
#include <cinatra.hpp>
#include <string>
#include <chrono>
namespace WebCFace {

void Client::messageThreadMain(std::shared_ptr<ClientData> data,
                               std::string host, int port) {
    using namespace cinatra;
    while (!data->closing.load()) {
        auto client = std::make_shared<coro_http_client>();

        // on_ws_closeが完了する前にclientが消えているとセグフォするので、
        // client_keep内にclientのshared_ptrをキープしておき
        // closeが完了したらその参照を消すことでやっとclientが消えるようにする
        auto client_keep =
            std::make_shared<std::shared_ptr<coro_http_client>>(client);

        data->connected_.store(false);
        std::weak_ptr<ClientData> data_w = data;
        auto connected_this = std::make_shared<std::atomic<bool>>(true);

        client->on_ws_msg(
            [data_w, connected_this, client_keep](resp_data rdata) {
                auto data = data_w.lock();
                if (rdata.net_err) {
                    if (data) {
                        data->logger_internal->error("recv error {}",
                                                     rdata.net_err.message());
                    }
                    connected_this->store(false);
                    client_keep->reset();
                }
                if (data) {
                    data->logger_internal->trace("message received");
                    data->recv_queue.push(std::string(rdata.resp_body));
                    data->logger_internal->trace("message recv done");
                }
            });
        client->on_ws_close([data_w, connected_this, client_keep](auto &&) {
            auto data = data_w.lock();
            if (data) {
                data->logger_internal->debug("connection closed");
            }
            connected_this->store(false);
            client_keep->reset();
        });

        data->logger_internal->trace("start connecting");
        bool ok = async_simple::coro::syncAwait(client->async_ws_connect(
            "ws://" + host + ":" + std::to_string(port)));
        data->logger_internal->trace("finished connecting, status={}", ok);
        if (ok) {
            data->sync_init.store(false);
            data->connected_.store(true);
            data->logger_internal->debug("connected");

            while (!data->closing.load() && data->connected_.load() &&
                   connected_this->load()) {
                auto msg =
                    data->message_queue.pop(std::chrono::milliseconds(10));
                if (msg) {
                    // this->send(*msg);
                    data->logger_internal->trace("sending message");
                    auto rdata = async_simple::coro::syncAwait(
                        client->async_send_ws(*msg, true, opcode::binary));
                    if (rdata.net_err) {
                        data->logger_internal->debug("send error {}",
                                                     rdata.net_err.message());
                        data->connected_.store(false);
                    }
                }
            }

            async_simple::coro::syncAwait(client->async_send_ws_close());
            data->logger_internal->trace("closing");
        }
    }
}

} // namespace WebCFace
#include <webcface/client.h>
#include <cinatra.hpp>
#include <string>
#include <chrono>
#include <thread>
namespace WebCFace {

void Client::messageThreadMain(std::shared_ptr<ClientData> data,
                               std::string host, int port) {
    while (!data->closing.load()) {
        using namespace cinatra;
        // 通信エラーで閉じる場合はdeleteするが、
        // 自分で閉じる場合は裏で受信スレッドがまだうごいているのでdeleteするとセグフォする
        auto client = new coro_http_client();
        std::weak_ptr<ClientData> data_w = data;
        auto connected_this = std::make_shared<std::atomic<bool>>(true);

        client->on_ws_msg([data_w, connected_this](resp_data rdata) {
            auto data = data_w.lock();
            if (rdata.net_err) {
                if (data) {
                    data->logger_internal->error("recv error {}",
                                                 rdata.net_err.message());
                }
                connected_this->store(false);
            } else {
                if (data) {
                    data->logger_internal->trace("message received");
                    data->recv_queue.push(std::string(rdata.resp_body));
                    data->logger_internal->trace("message recv done");
                }
            }
        });
        client->on_ws_close([data_w, connected_this](auto &&) {
            auto data = data_w.lock();
            if (data) {
                data->logger_internal->debug("connection closed");
            }
            connected_this->store(false);
        });


        connected_this->store(true);
        data->connected_.store(false);

        data->logger_internal->trace("start connecting");
        bool ok = async_simple::coro::syncAwait(client->async_ws_connect(
            "ws://" + host + ":" + std::to_string(port)));
        if (ok) {
            data->sync_init.store(false);
            data->connected_.store(true);
            data->logger_internal->debug("connected");

            while (!data->closing.load() && data->connected_.load() &&
                   connected_this->load()) {
                auto msg =
                    data->message_queue.pop(std::chrono::milliseconds(10));
                if (msg) {
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

            if (!connected_this->load()) {
                delete client;
            }
        } else {
            delete client;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

} // namespace WebCFace
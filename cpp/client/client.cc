#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>
#include <string>
#include <webcface/webcface.h>
#include "../message/message.h"

namespace WebCFace {

Client::Client(const std::string &name, const std::string &host, int port) {
    using namespace drogon;
    ws = WebSocketClient::newWebSocketClient(host, port, false);
    auto req = HttpRequest::newHttpRequest();
    req->setPath("/");

    ws->setMessageHandler(
        [this](const std::string &message, const WebSocketClientPtr &ws,
               const WebSocketMessageType &type) { onRecv(message); });
    ws->setConnectionClosedHandler(
        [](const WebSocketClientPtr &ws) { std::cout << "closed\n"; });

    ws->connectToServer(req,
                        [name, this](ReqResult r, const HttpResponsePtr &resp,
                                     const WebSocketClientPtr &ws) {
                            auto c = ws->getConnection();
                            if (r == ReqResult::Ok) {
                                connected = true;
                                std::cout << "connected\n";
                                c->send(Message::pack(Message::Name{{}, name}));
                            } else {
                                std::cout << "error\n";
                                // todo: エラー時どうするか
                            }
                        });
}

void Client::send() {
    if (connected) {
        auto c = ws->getConnection();

        auto value_send = value_store->transfer_send();
        for (const auto &v : value_send) {
            c->send(Message::pack(Message::Value{{}, v.first, v.second}));
        }
        auto value_subsc = value_store->transfer_subsc();
        for (const auto &v : value_subsc) {
            c->send(Message::pack(
                Message::Subscribe<Message::Value>{{}, v.first, v.second}));
        }
        auto text_send = text_store->transfer_send();
        for (const auto &v : text_send) {
            c->send(Message::pack(Message::Text{{}, v.first, v.second}));
        }
        auto text_subsc = text_store->transfer_subsc();
        for (const auto &v : text_subsc) {
            c->send(Message::pack(
                Message::Subscribe<Message::Text>{{}, v.first, v.second}));
        }
    }
}
void Client::onRecv(const std::string &message) {
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
    switch (kind) {
    case kind_recv(MessageKind::value): {
        auto r = std::any_cast<Recv<WebCFace::Message::Value>>(obj);
        value_store->set_recv(r.from, r.name, r.data);
        break;
    }
    case kind_recv(MessageKind::text): {
        auto r = std::any_cast<Recv<WebCFace::Message::Text>>(obj);
        text_store->set_recv(r.from, r.name, r.data);
        break;
    }
    case MessageKind::call: {
        auto r = std::any_cast<Call>(obj);
        auto res = this->func(r.name).run_impl(r.args).get();
        std::string response;
        if (res.is_error) {
            response = res.error_msg;
        } else {
            response = static_cast<std::string>(res.result);
        }
        // todo: これセグフォしない?
        ws->getConnection()->send(pack(CallResponse{
            {}, r.caller_id, r.caller, res.found, res.is_error, response}));
        break;
    }
    case MessageKind::call_response: {
        auto r = std::any_cast<CallResponse>(obj);
        auto res = func_impl_store->getResult(r.caller_id);
        res.found = r.found;
        res.is_error = r.is_error;
        if (r.is_error) {
            res.error_msg = r.response;
        } else {
            res.result = static_cast<AnyArg>(r.response);
        }
        res.setReady();
        break;
    }
    }
}

static struct MainLoop {
    std::optional<std::thread> thr;
    MainLoop() {
        using namespace drogon;
        std::cout << "mainloop start" << std::endl;
        std::promise<void> p1;
        std::future<void> f1 = p1.get_future();

        // Start the main loop on another thread
        thr = std::make_optional<std::thread>([&]() {
            // Queues the promise to be fulfilled after starting the loop
            app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
            app().run();
        });

        // The future is only satisfied after the event loop started
        f1.get();
        std::cout << "thread started" << std::endl;
    }
    ~MainLoop() {
        using namespace drogon;
        app().getLoop()->queueInLoop([]() { app().quit(); });
        std::cout << "mainloop quit" << std::endl;
        thr->join();
        std::cout << "thread finished" << std::endl;
    }
} q;
} // namespace WebCFace
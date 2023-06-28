#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>
#include <string>
#include <chrono>
#include <thread>
#include <future>
#include <webcface/webcface.h>
#include "../message/message.h"

namespace WebCFace {

Client::Client(const std::string &name, const std::string &host, int port) : name(name), host(host), port(port){
    using namespace drogon;
    reconnect();
}
bool Client::connected() const {
    return ws->getConnection() && ws->getConnection()->connected();
}
void Client::reconnect() {
    if (!closing) {
        using namespace drogon;
        ws = WebSocketClient::newWebSocketClient(host, port, false);
        ws->setMessageHandler(
            [this](const std::string &message, const WebSocketClientPtr &ws,
                   const WebSocketMessageType &type) { onRecv(message); });
        ws->setConnectionClosedHandler([this](const WebSocketClientPtr &ws) {
            std::cout << "closed" << std::endl;
            reconnect();
        });
        auto req = HttpRequest::newHttpRequest();
        req->setPath("/");
        auto p = std::make_shared<std::promise<void>>();
        connection_finished = p->get_future();
        ws->connectToServer(
            req, [this, p](ReqResult r, const HttpResponsePtr &resp,
                           const WebSocketClientPtr &ws) mutable {
                auto c = ws->getConnection();
                if (r == ReqResult::Ok) {
                    std::cout << "connected\n";
                    send(Message::pack(Message::Name{{}, name}));
                } else {
                    std::cout << "error\n";
                    // todo: エラー時どうするか
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    reconnect();
                }
                p->set_value();
            });
    }
}
Client::~Client() {
    close();
    // reconnectが終了していなければ待機する
    if (connection_finished.valid()) {
        connection_finished.get();
    }
}
void Client::close() {
    closing = true;
    if (ws->getConnection()) {
        ws->getConnection()->shutdown();
    }
}
void Client::send(const std::vector<char> &m) {
    if (connected()) {
        ws->getConnection()->send(&m[0], m.size(),
                                  drogon::WebSocketMessageType::Binary);
    }
}
void Client::send() {
    if (connected()) {
        auto value_send = value_store->transfer_send();
        for (const auto &v : value_send) {
            send(Message::pack(Message::Value{{}, v.first, v.second}));
        }
        auto value_subsc = value_store->transfer_subsc();
        for (const auto &v : value_subsc) {
            send(Message::pack(
                Message::Subscribe<Message::Value>{{}, v.first, v.second}));
        }
        auto text_send = text_store->transfer_send();
        for (const auto &v : text_send) {
            send(Message::pack(Message::Text{{}, v.first, v.second}));
        }
        auto text_subsc = text_store->transfer_subsc();
        for (const auto &v : text_subsc) {
            send(Message::pack(
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
        send(pack(CallResponse{
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
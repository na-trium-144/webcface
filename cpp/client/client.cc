#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>
#include <string>
#include <future>
#include <chrono>
#include <webcface/webcface.h>
#include "../message/message.h"

namespace WebCFace {

Client::Client(const std::string &name, const std::string &host, int port)
    : name(name), host(host), port(port) {

    // 最初のクライアント接続時にdrogonループを起動
    // もしClientがテンプレートクラスになったら使えない
    static struct MainLoop {
        std::optional<std::thread> thr;
        MainLoop() {
            using namespace drogon;
            std::cout << "mainloop start" << std::endl;
            std::promise<void> p1;
            std::future<void> f1 = p1.get_future();

            app().disableSigtermHandling();
            app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
            // Start the main loop on another thread
            thr = std::make_optional<std::thread>([&]() {
                // Queues the promise to be fulfilled after starting the loop
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

    reconnect();
}

bool Client::connected() const {
    return ws && ws->getConnection() && ws->getConnection()->connected();
}
void Client::reconnect() {
    if (ws && ws->getConnection()) {
        ws->getConnection()->shutdown();
    }
    if (!closing) {
        std::cout << "reconnect" << std::endl;
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
        auto p_cli = std::make_shared<std::promise<void>>();
        connection_finished = p_cli->get_future();
        auto p_local = std::make_shared<std::promise<void>>();
        ws->connectToServer(req, [this, p_cli, p_local](
                                     ReqResult r, const HttpResponsePtr &resp,
                                     const WebSocketClientPtr &ws) mutable {
            if (r == ReqResult::Ok) {
                std::cout << "connected " << std::endl;
                send(Message::pack(Message::Name{{}, name}));
            } else {
                std::cout << "error " << r << std::endl;
                app().getLoop()->runAfter(1, [this] { reconnect(); });
            }
            p_cli->set_value();
            p_local->set_value();
        });
        app().getLoop()->runAfter(1, [this, p_local] {
            if (p_local->get_future().wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready) {
                std::cout << "timeout!" << std::endl;
                reconnect();
            }
        });
    }
}
Client::~Client() {
    close();
    // reconnectが終了していなければ待機する
    if (connection_finished.valid()) {
        connection_finished.wait();
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
        auto value_send = value_store->transferSend();
        for (const auto &v : value_send) {
            send(Message::pack(Message::Value{{}, v.first, v.second}));
        }
        auto value_subsc = value_store->transferReq();
        for (const auto &v : value_subsc) {
            for (const auto &v2 : v.second) {
                send(Message::pack(
                    Message::Subscribe<Message::Value>{{}, v.first, v2.first}));
            }
        }
        auto text_send = text_store->transferSend();
        for (const auto &v : text_send) {
            send(Message::pack(Message::Text{{}, v.first, v.second}));
        }
        auto text_subsc = text_store->transferReq();
        for (const auto &v : text_subsc) {
            for (const auto &v2 : v.second) {
                send(Message::pack(
                    Message::Subscribe<Message::Text>{{}, v.first, v2.first}));
            }
        }
    }
}
void Client::onRecv(const std::string &message) {
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch (kind) {
    case kind_recv(MessageKind::value): {
        auto r = std::any_cast<Recv<WebCFace::Message::Value>>(obj);
        value_store->setRecv(r.from, r.name, r.data);
        break;
    }
    case kind_recv(MessageKind::text): {
        auto r = std::any_cast<Recv<WebCFace::Message::Text>>(obj);
        text_store->setRecv(r.from, r.name, r.data);
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
    case MessageKind::entry: {
        auto r = std::any_cast<Message::Entry>(obj);
        std::vector<std::string> values(r.value.size());
        std::vector<std::string> texts(r.text.size());
        for (std::size_t i = 0; i < r.value.size(); i++) {
            values[i] = r.value[i].name;
        }
        for (std::size_t i = 0; i < r.text.size(); i++) {
            texts[i] = r.text[i].name;
        }
        value_store->setEntry(r.name, values);
        text_store->setEntry(r.name, texts);
        break;
    }
    case MessageKind::name:
    case MessageKind::value:
    case MessageKind::text:
        std::cerr << "Invalid Message Kind " << static_cast<int>(kind)
                  << std::endl;
        break;
    default:
        std::cerr << "Unknown Message Kind " << static_cast<int>(kind)
                  << std::endl;
        break;
    }
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
}
} // namespace WebCFace
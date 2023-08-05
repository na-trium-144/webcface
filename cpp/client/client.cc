#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>
#include <string>
#include <future>
#include <chrono>
#include <iostream>
#include <webcface/webcface.h>
#include "../message/message.h"

namespace WebCFace {

Client::Client(const std::string &name, const std::string &host, int port)
    : data(std::make_shared<ClientData>()), self_(data, ""), name_(name),
      host(host), port(port), event_thread([this] {
          while (!closing.load()) {
              data->event_queue.waitFor(std::chrono::milliseconds(10));
              data->event_queue.process();
          }
      }),
      func_call_thread([this] {
          while (!closing.load()) {
              auto call =
                  data->func_call_queue.pop(std::chrono::milliseconds(10));
              if (call) {
                  this->send(Message::pack(Message::Call{*call}));
              }
          }
      }) {

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
    if (!closing.load()) {
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
                send(Message::pack(Message::Name{{}, name_}));
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
    event_thread.join();
    func_call_thread.join();
}
void Client::close() {
    closing.store(true);
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
        auto value_send = data->value_store.transferSend();
        for (const auto &v : value_send) {
            send(Message::pack(Message::Value{{}, "", v.first, v.second}));
        }
        auto value_subsc = data->value_store.transferReq();
        for (const auto &v : value_subsc) {
            for (const auto &v2 : v.second) {
                send(Message::pack(
                    Message::Subscribe<Message::Value>{{}, v.first, v2.first}));
            }
        }
        auto text_send = data->text_store.transferSend();
        for (const auto &v : text_send) {
            send(Message::pack(Message::Text{{}, "", v.first, v.second}));
        }
        auto text_subsc = data->text_store.transferReq();
        for (const auto &v : text_subsc) {
            for (const auto &v2 : v.second) {
                send(Message::pack(
                    Message::Subscribe<Message::Text>{{}, v.first, v2.first}));
            }
        }
        auto func_send = data->func_store.transferSend();
        for (const auto &v : func_send) {
            send(Message::pack(Message::FuncInfo{"", v.first, v.second}));
        }
    }
}
void Client::onRecv(const std::string &message) {
    using MessageKind = WebCFace::Message::MessageKind;
    auto [kind, obj] = WebCFace::Message::unpack(message);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
    switch (kind) {
    case MessageKind::value: {
        auto r = std::any_cast<WebCFace::Message::Value>(obj);
        data->value_store.setRecv(r.member, r.name, r.data);
        data->event_queue.enqueue(
            EventKey{EventType::value_change, r.member, r.name}, data);
        break;
    }
    case MessageKind::text: {
        auto r = std::any_cast<WebCFace::Message::Text>(obj);
        data->text_store.setRecv(r.member, r.name, r.data);
        data->event_queue.enqueue(
            EventKey{EventType::text_change, r.member, r.name}, data);
        break;
    }
    case MessageKind::call: {
        auto r = std::any_cast<WebCFace::Message::Call>(obj);
        auto &async_res = self().func(r.name).runAsync(r.args);
        std::thread([this, &async_res, r] {
            bool started = async_res.started.get();
            send(WebCFace::Message::pack(WebCFace::Message::CallResponse{
                {}, r.caller_id, r.caller, started}));
            if (started) {
                bool is_error = false;
                std::string result;
                try {
                    result = async_res.result.get();
                } catch (const std::exception &e) {
                    is_error = true;
                    result = e.what();
                }
                send(WebCFace::Message::pack(WebCFace::Message::CallResult{
                    {}, r.caller_id, r.caller, is_error, result}));
            }
        }).detach();
        break;
    }
    case MessageKind::call_response: {
        auto r = std::any_cast<WebCFace::Message::CallResponse>(obj);
        auto &res = data->func_result_store.getResult(r.caller_id);
        res.started_->set_value(r.started);
        if (!r.started) {
            try {
                throw FuncNotFound(res.member_, res.name_);
            } catch (...) {
                res.result_->set_exception(std::current_exception());
            }
        }
        break;
    }
    case MessageKind::call_result: {
        auto r = std::any_cast<WebCFace::Message::CallResult>(obj);
        auto &res = data->func_result_store.getResult(r.caller_id);
        if (r.is_error) {
            try {
                throw std::runtime_error(r.result);
            } catch (...) {
                res.result_->set_exception(std::current_exception());
            }
        } else {
            // todo: 戻り値の型?
            res.result_->set_value(ValAdaptor{r.is_error ? "" : r.result});
        }
        break;
    }
    case MessageKind::name: {
        auto r = std::any_cast<WebCFace::Message::Name>(obj);
        data->value_store.setEntry(r.name);
        data->text_store.setEntry(r.name);
        data->func_store.setEntry(r.name);
        data->event_queue.enqueue(EventKey{EventType::member_entry, r.name}, data);
        break;
    }
    case kind_entry(MessageKind::value): {
        auto r =
            std::any_cast<WebCFace::Message::Entry<WebCFace::Message::Value>>(
                obj);
        data->value_store.setEntry(r.member, r.name);
        data->event_queue.enqueue(
            EventKey{EventType::value_entry, r.member, r.name}, data);
        break;
    }
    case kind_entry(MessageKind::text): {
        auto r =
            std::any_cast<WebCFace::Message::Entry<WebCFace::Message::Text>>(
                obj);
        data->text_store.setEntry(r.member, r.name);
        data->event_queue.enqueue(
            EventKey{EventType::text_entry, r.member, r.name}, data);
        break;
    }
    case MessageKind::func_info: {
        auto r = std::any_cast<WebCFace::Message::FuncInfo>(obj);
        data->func_store.setEntry(r.member, r.name);
        data->func_store.setRecv(r.member, r.name, static_cast<FuncInfo>(r));
        data->event_queue.enqueue(
            EventKey{EventType::func_entry, r.member, r.name}, data);
        break;
    }
    // case :
    //     std::cerr << "Invalid Message Kind " << static_cast<int>(kind)
    //               << std::endl;
    //     break;
    case MessageKind::unknown:
        break;
    default:
        std::cerr << "Unknown Message Kind " << static_cast<int>(kind)
                  << std::endl;
        break;
    }
#pragma GCC diagnostic pop
}
} // namespace WebCFace
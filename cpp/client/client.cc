#include <webcface/client.h>
#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>
#include <string>
#include <chrono>
#include <cstdio>
#include "../message/message.h"

namespace WebCFace {

Client::Client(const std::string &name, const std::string &host, int port)
    : Member(), data(std::make_shared<ClientData>(name)), host(host),
      port(port), event_thread([this] {
          while (!closing.load()) {
              data->event_queue.waitFor(std::chrono::milliseconds(10));
              data->event_queue.process();
          }
      }),
      message_thread([this] {
          while (!closing.load()) {
              auto msg = data->message_queue.pop(std::chrono::milliseconds(10));
              if (msg) {
                  this->send(*msg);
              }
          }
      }),
      logger_buf(this->data), logger_os(&this->logger_buf) {

    this->Member::data_w = this->data;
    this->Member::member_ = name;

    // 最初のクライアント接続時にdrogonループを起動
    // もしClientがテンプレートクラスになったら使えない
    static struct MainLoop {
        std::optional<std::thread> thr;
        std::weak_ptr<ClientData> data_w;
        MainLoop(const std::weak_ptr<ClientData> &data_w) : data_w(data_w) {
            using namespace drogon;
            if (auto data = data_w.lock()) {
                data->logger_internal->trace("mainloop start");
            }

            auto log_output_func = [data_w](const char *msg, uint64_t len) {
                if (auto data = data_w.lock()) {
                    // 末尾の改行を除く
                    if (len > 0 && msg[len - 1] == '\n') {
                        --len;
                    }
                    std::string log(msg, len);
                    std::stringstream ss(log);
                    std::string level;
                    ss >> level >> level >> level >> level >> level;
                    log = "from drogon: " + log;
                    if (level == "TRACE") {
                        data->logger_internal->trace(log);
                    } else if (level == "DEBUG") {
                        data->logger_internal->debug(log);
                    } else if (level == "INFO") {
                        data->logger_internal->info(log);
                    } else if (level == "ERROR") {
                        data->logger_internal->error(log);
                    } else if (level == "FATAL") {
                        data->logger_internal->critical(log);
                    } else {
                        // default
                        data->logger_internal->warn(log);
                    }
                }
            };
            auto log_flush_func = [] {};
            trantor::Logger::setOutputFunction(log_output_func, log_flush_func);

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
            if (auto data = data_w.lock()) {
                data->logger_internal->trace("thread started");
            }
        }
        ~MainLoop() {
            using namespace drogon;
            app().getLoop()->queueInLoop([]() { app().quit(); });
            if (auto data = data_w.lock()) {
                data->logger_internal->trace("mainloop quit");
            }
            thr->join();
            if (auto data = data_w.lock()) {
                data->logger_internal->trace("thread finished");
            }
        }
    } q{this->data};

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
        data->logger_internal->trace("reconnect");
        using namespace drogon;
        ws = WebSocketClient::newWebSocketClient(host, port, false);
        ws->setMessageHandler(
            [this](const std::string &message, const WebSocketClientPtr &ws,
                   const WebSocketMessageType &type) { onRecv(message); });
        ws->setConnectionClosedHandler([this](const WebSocketClientPtr &ws) {
            data->logger_internal->debug("closed");
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
                data->logger_internal->info("connected");
            } else {
                data->logger_internal->warn("connection error {}", r);
                app().getLoop()->runAfter(1, [this] { reconnect(); });
            }
            p_cli->set_value();
            p_local->set_value();
        });
        app().getLoop()->runAfter(1, [this, p_local] {
            if (p_local->get_future().wait_for(std::chrono::seconds(0)) !=
                std::future_status::ready) {
                data->logger_internal->warn("connection timeout");
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
    message_thread.join();
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
void Client::sync() {
    if (connected()) {
        if (!sync_init) {
            send(Message::pack(Message::SyncInit{{}, member_}));
            sync_init = true;
        }

        auto value_send = data->value_store.transferSend();
        for (const auto &v : value_send) {
            send(Message::pack(Message::Value{{}, "", v.first, v.second}));
        }
        auto value_subsc = data->value_store.transferReq();
        for (const auto &v : value_subsc) {
            for (const auto &v2 : v.second) {
                send(Message::pack(
                    Message::Req<Message::Value>{{}, v.first, v2.first}));
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
                    Message::Req<Message::Text>{{}, v.first, v2.first}));
            }
        }
        auto view_send_prev = data->view_store.getSendPrev();
        auto view_send = data->view_store.transferSend();
        for (const auto &v : view_send) {
            auto v_prev = view_send_prev.find(v.first);
            std::unordered_map<int, ViewComponent> v_diff;
            if (v_prev == view_send_prev.end()) {
                for (std::size_t i = 0; i < v.second.size(); i++) {
                    v_diff[i] = v.second[i];
                }
            } else {
                v_diff = getViewDiff(v.second, v_prev->second);
            }
            if (!v_diff.empty()) {
                send(Message::pack(Message::View{
                    "", v.first, v_diff, static_cast<int>(v.second.size())}));
            }
        }
        auto view_subsc = data->view_store.transferReq();
        for (const auto &v : view_subsc) {
            for (const auto &v2 : v.second) {
                send(Message::pack(
                    Message::Req<Message::View>{{}, v.first, v2.first}));
            }
        }
        auto func_send = data->func_store.transferSend();
        for (const auto &v : func_send) {
            send(Message::pack(Message::FuncInfo{"", v.first, v.second}));
        }

        auto log_subsc = data->log_store.transferReq();
        for (const auto &v : log_subsc) {
            send(Message::pack(Message::LogReq{{}, v.first}));
        }
        std::vector<Message::Log::LogLine> log_send;
        while (auto log = data->logger_sink->pop()) {
            log_send.push_back(*log);
            // todo: connected状態でないとlog_storeにログが記録されない
            data->log_store.addRecv(this->name(), *log);
        }
        if (!log_send.empty()) {
            send(Message::pack(Message::Log{{}, "", log_send}));
        }
    }
    while (auto func_sync = data->func_sync_queue.pop()) {
        (*func_sync)->sync();
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
        data->value_store.setRecv(r.member, r.field, r.data);
        data->event_queue.enqueue(
            EventKey{EventType::value_change, Field{data, r.member, r.field}});
        break;
    }
    case MessageKind::text: {
        auto r = std::any_cast<WebCFace::Message::Text>(obj);
        data->text_store.setRecv(r.member, r.field, r.data);
        data->event_queue.enqueue(
            EventKey{EventType::text_change, Field{data, r.member, r.field}});
        break;
    }
    case MessageKind::view: {
        auto r = std::any_cast<WebCFace::Message::View>(obj);
        auto v_prev = data->view_store.getRecv(r.member, r.field);
        if (v_prev == std::nullopt) {
            v_prev = {};
        }
        mergeViewDiff(r.data_diff, r.length, *v_prev);
        data->view_store.setRecv(r.member, r.field, *v_prev);
        data->event_queue.enqueue(
            EventKey{EventType::view_change, Field{data, r.member, r.field}});
        break;
    }
    case MessageKind::log: {
        auto r = std::any_cast<WebCFace::Message::Log>(obj);
        for (const auto &lm : r.log) {
            data->log_store.addRecv(r.member, lm);
        }
        data->event_queue.enqueue(
            EventKey{EventType::log_change, Field{data, r.member}});
        break;
    }
    case MessageKind::call: {
        auto r = std::any_cast<WebCFace::Message::Call>(obj);
        std::thread([data = this->data, r] {
            auto func_info =
                data->func_store.getRecv(data->self_member_name, r.field);
            if (func_info) {
                data->message_queue.push(
                    WebCFace::Message::pack(WebCFace::Message::CallResponse{
                        {}, r.caller_id, r.caller, true}));
                ValAdaptor result;
                bool is_error = false;
                try {
                    result = func_info->run(r.args);
                } catch (const std::exception &e) {
                    is_error = true;
                    result = e.what();
                } catch (const std::string &e) {
                    is_error = true;
                    result = e;
                } catch (...) {
                    is_error = true;
                    result = "unknown exception";
                }
                data->message_queue.push(
                    WebCFace::Message::pack(WebCFace::Message::CallResult{
                        {}, r.caller_id, r.caller, is_error, result}));
            } else {
                data->message_queue.push(
                    WebCFace::Message::pack(WebCFace::Message::CallResponse{
                        {}, r.caller_id, r.caller, false}));
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
                throw FuncNotFound(res);
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
                throw std::runtime_error(static_cast<std::string>(r.result));
            } catch (...) {
                res.result_->set_exception(std::current_exception());
            }
        } else {
            // todo: 戻り値の型?
            res.result_->set_value(ValAdaptor{r.result});
        }
        break;
    }
    case MessageKind::sync_init: {
        auto r = std::any_cast<WebCFace::Message::SyncInit>(obj);
        data->value_store.setEntry(r.member);
        data->text_store.setEntry(r.member);
        data->func_store.setEntry(r.member);
        data->event_queue.enqueue(
            EventKey{EventType::member_entry, Field{data, r.member}});
        break;
    }
    case kind_entry(MessageKind::value): {
        auto r =
            std::any_cast<WebCFace::Message::Entry<WebCFace::Message::Value>>(
                obj);
        data->value_store.setEntry(r.member, r.field);
        data->event_queue.enqueue(
            EventKey{EventType::value_entry, Field{data, r.member, r.field}});
        break;
    }
    case kind_entry(MessageKind::text): {
        auto r =
            std::any_cast<WebCFace::Message::Entry<WebCFace::Message::Text>>(
                obj);
        data->text_store.setEntry(r.member, r.field);
        data->event_queue.enqueue(
            EventKey{EventType::text_entry, Field{data, r.member, r.field}});
        break;
    }
    case kind_entry(MessageKind::view): {
        auto r =
            std::any_cast<WebCFace::Message::Entry<WebCFace::Message::View>>(
                obj);
        data->view_store.setEntry(r.member, r.field);
        data->event_queue.enqueue(
            EventKey{EventType::view_entry, Field{data, r.member, r.field}});
        break;
    }
    case MessageKind::func_info: {
        auto r = std::any_cast<WebCFace::Message::FuncInfo>(obj);
        data->func_store.setEntry(r.member, r.field);
        data->func_store.setRecv(r.member, r.field, static_cast<FuncInfo>(r));
        data->event_queue.enqueue(
            EventKey{EventType::func_entry, Field{data, r.member, r.field}});
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
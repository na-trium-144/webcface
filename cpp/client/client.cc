#include <webcface/client.h>
#include <string>
#include <chrono>
#include <iostream>
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
      message_thread([this] { this->messageThreadMain(); }),
      logger_buf(this->data), logger_os(&this->logger_buf) {

    this->Member::data_w = this->data;
    this->Member::member_ = name;
}

Client::~Client() {
    close();
    event_thread.join();
    message_thread.join();
}
void Client::close() { closing.store(true); }

void Client::sync() {
    if (connected()) {
        std::stringstream buffer;
        int len = 0;
        Message::pack(buffer, len, Message::Sync{});

        bool is_first = false;
        if (!sync_init.load()) {
            Message::pack(buffer, len, Message::SyncInit{{}, member_});
            is_first = true;
            sync_init.store(true);
        }

        // todo: hiddenの反映
        for (const auto &v : data->value_store.transferSend(is_first)) {
            Message::pack(buffer, len, Message::Value{{}, v.first, v.second});
        }
        for (const auto &v : data->value_store.transferReq(is_first)) {
            for (const auto &v2 : v.second) {
                Message::pack(buffer, len,
                              Message::Req<Message::Value>{
                                  {}, v.first, v2.first, v2.second});
            }
        }
        for (const auto &v : data->text_store.transferSend(is_first)) {

            Message::pack(buffer, len, Message::Text{{}, v.first, v.second});
        }
        for (const auto &v : data->text_store.transferReq(is_first)) {
            for (const auto &v2 : v.second) {
                Message::pack(buffer, len,
                              Message::Req<Message::Text>{
                                  {}, v.first, v2.first, v2.second});
            }
        }
        auto view_send_prev = data->view_store.getSendPrev(is_first);
        auto view_send = data->view_store.transferSend(is_first);
        for (const auto &v : view_send) {
            auto v_prev = view_send_prev.find(v.first);
            std::unordered_map<int, ViewComponentBase> v_diff;
            if (v_prev == view_send_prev.end()) {
                for (std::size_t i = 0; i < v.second.size(); i++) {
                    v_diff[i] = v.second[i];
                }
            } else {
                for (std::size_t i = 0; i < v.second.size(); i++) {
                    if (v_prev->second.size() <= i ||
                        v_prev->second[i] != v.second[i]) {
                        v_diff[i] = v.second[i];
                    }
                }
            }
            if (!v_diff.empty()) {
                Message::pack(buffer, len,
                              Message::View{v.first, v_diff,
                                            static_cast<int>(v.second.size())});
            }
        }
        for (const auto &v : data->view_store.transferReq(is_first)) {
            for (const auto &v2 : v.second) {
                Message::pack(buffer, len,
                              Message::Req<Message::View>{
                                  {}, v.first, v2.first, v2.second});
            }
        }
        for (const auto &v : data->func_store.transferSend(is_first)) {
            if (!data->func_store.isHidden(v.first)) {
                Message::pack(buffer, len,
                              Message::FuncInfo{v.first, v.second});
            }
        }

        for (const auto &v : data->log_store.transferReq(is_first)) {
            Message::pack(buffer, len, Message::LogReq{{}, v.first});
        }

        std::vector<Message::Log::LogLine> log_send;
        while (auto log = data->logger_sink->pop()) {
            log_send.push_back(*log);
            // todo: connected状態でないとlog_storeにログが記録されない
            data->log_store.addRecv(this->name(), *log);
        }
        if (is_first) {
            auto log_send_m = data->log_store.getRecv(member_).value_or(
                std::vector<LogLine>{});
            log_send.resize(log_send_m.size());
            for (std::size_t i = 0; i < log_send_m.size(); i++) {
                log_send[i] = log_send_m[i];
            }
        }
        if (!log_send.empty()) {
            Message::pack(buffer, len, Message::Log{{}, log_send});
        }

        data->message_queue.push(Message::packDone(buffer, len));
    }
    while (auto func_sync = data->func_sync_queue.pop()) {
        (*func_sync)->sync();
    }
}
void Client::onRecv(const std::string &message) {
    namespace MessageKind = WebCFace::Message::MessageKind;
    auto messages = WebCFace::Message::unpack(message);
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::sync: {
            auto r = std::any_cast<WebCFace::Message::Sync>(obj);
            data->sync_time_store.setRecv(r.member, r.getTime());
        }
        case MessageKind::value + MessageKind::res: {
            auto r =
                std::any_cast<WebCFace::Message::Res<WebCFace::Message::Value>>(
                    obj);
            auto [member, field] = data->value_store.getReq(r.req_id);
            data->value_store.setRecv(member, field, r.data);
            data->event_queue.enqueue(
                EventKey{EventType::value_change, Field{data, member, field}});
            break;
        }
        case MessageKind::text + MessageKind::res: {
            auto r =
                std::any_cast<WebCFace::Message::Res<WebCFace::Message::Text>>(
                    obj);
            auto [member, field] = data->text_store.getReq(r.req_id);
            data->text_store.setRecv(member, field, r.data);
            data->event_queue.enqueue(
                EventKey{EventType::text_change, Field{data, member, field}});
            break;
        }
        case MessageKind::view + MessageKind::res: {
            auto r =
                std::any_cast<WebCFace::Message::Res<WebCFace::Message::View>>(
                    obj);
            auto [member, field] = data->view_store.getReq(r.req_id);
            auto v_prev = data->view_store.getRecv(member, field);
            if (v_prev == std::nullopt) {
                v_prev = {};
            }
            v_prev->resize(r.length);
            for (const auto &d : r.data_diff) {
                v_prev->at(d.first) = d.second;
            }
            data->view_store.setRecv(member, field, *v_prev);
            data->event_queue.enqueue(
                EventKey{EventType::view_change, Field{data, member, field}});
            break;
        }
        case MessageKind::log: {
            auto r = std::any_cast<WebCFace::Message::Log>(obj);
            for (const auto &lm : r.log) {
                data->log_store.addRecv(r.member, static_cast<LogLine>(lm));
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
                    data->message_queue.push(WebCFace::Message::packSingle(
                        WebCFace::Message::CallResponse{
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

                    data->message_queue.push(WebCFace::Message::packSingle(
                        WebCFace::Message::CallResult{
                            {}, r.caller_id, r.caller, is_error, result}));
                } else {

                    data->message_queue.push(WebCFace::Message::packSingle(
                        WebCFace::Message::CallResponse{
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
                    throw std::runtime_error(
                        static_cast<std::string>(r.result));
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
        case MessageKind::entry + MessageKind::value: {
            auto r = std::any_cast<
                WebCFace::Message::Entry<WebCFace::Message::Value>>(obj);
            data->value_store.setEntry(sync_member, r.field);
            data->event_queue.enqueue(EventKey{EventType::value_entry,
                                               Field{data, r.member, r.field}});
            break;
        }
        case MessageKind::entry + MessageKind::text: {
            auto r = std::any_cast<
                WebCFace::Message::Entry<WebCFace::Message::Text>>(obj);
            data->text_store.setEntry(r.member, r.field);
            data->event_queue.enqueue(EventKey{EventType::text_entry,
                                               Field{data, r.member, r.field}});
            break;
        }
        case MessageKind::entry + MessageKind::view: {
            auto r = std::any_cast<
                WebCFace::Message::Entry<WebCFace::Message::View>>(obj);
            data->view_store.setEntry(r.member, r.field);
            data->event_queue.enqueue(EventKey{EventType::view_entry,
                                               Field{data, r.member, r.field}});
            break;
        }
        case MessageKind::func_info: {
            auto r = std::any_cast<WebCFace::Message::FuncInfo>(obj);
            data->func_store.setEntry(r.member, r.field);
            data->func_store.setRecv(r.member, r.field,
                                     static_cast<FuncInfo>(r));
            data->event_queue.enqueue(EventKey{EventType::func_entry,
                                               Field{data, r.member, r.field}});
            break;
        }
        case MessageKind::value:
        case MessageKind::text:
        case MessageKind::view:
            std::cerr << "Invalid Message Kind " << static_cast<int>(kind)
                      << std::endl;
            break;
        case MessageKind::unknown:
            break;
        default:
            std::cerr << "Unknown Message Kind " << static_cast<int>(kind)
                      << std::endl;
            break;
        }
    }
}
} // namespace WebCFace
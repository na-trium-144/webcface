#include <webcface/client.h>
#include <webcface/data.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <string>
#include <chrono>
#include "../message/message.h"

namespace WebCFace {

Client::Client(const std::string &name, const std::string &host, int port)
    : Member(), data(std::make_shared<ClientData>(name)), host(host),
      port(port), message_thread([this] { this->messageThreadMain(); }),
      logger_buf(this->data), logger_os(&this->logger_buf) {

    this->Member::data_w = this->data;
    this->Member::member_ = name;
}

Client::~Client() {
    close();
    message_thread.join();
}
void Client::close() { closing.store(true); }

void Client::sync() {
    if (connected()) {
        std::stringstream buffer;
        int len = 0;

        bool is_first = false;
        if (!sync_init.load()) {
            Message::pack(buffer, len, Message::SyncInit{{}, member_, 0});
            is_first = true;
            sync_init.store(true);
        }

        Message::pack(buffer, len, Message::Sync{});

        // todo: hiddenの反映
        for (const auto &v : data->value_store.transferSend(is_first)) {
            Message::pack(
                buffer, len,
                Message::Value{
                    {},
                    v.first,
                    std::static_pointer_cast<std::vector<double>>(v.second)});
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
            auto v_diff =
                std::make_shared<std::unordered_map<int, ViewComponentBase>>();
            if (v_prev == view_send_prev.end()) {
                for (std::size_t i = 0; i < v.second->size(); i++) {
                    v_diff->emplace(i, (*v.second)[i]);
                }
            } else {
                for (std::size_t i = 0; i < v.second->size(); i++) {
                    if (v_prev->second->size() <= i ||
                        (*v_prev->second)[i] != (*v.second)[i]) {
                        v_diff->emplace(i, (*v.second)[i]);
                    }
                }
            }
            if (!v_diff->empty()) {
                Message::pack(buffer, len,
                              Message::View{v.first, v_diff, v.second->size()});
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
                              Message::FuncInfo{v.first, *v.second});
            }
        }

        for (const auto &v : data->log_store.transferReq(is_first)) {
            Message::pack(buffer, len, Message::LogReq{{}, v.first});
        }

        auto log_s = data->log_store.getRecv(member_);
        if (!log_s) {
            log_s = std::make_shared<std::vector<LogLine>>();
            data->log_store.setRecv(member_, *log_s);
        }
        auto log_send = std::make_shared<std::vector<Message::Log::LogLine>>();
        if (is_first) {
            log_send->resize((*log_s)->size());
            for (std::size_t i = 0; i < (*log_s)->size(); i++) {
                (*log_send)[i] = (**log_s)[i];
            }
        }
        while (auto log = data->logger_sink->pop()) {
            log_send->push_back(*log);
            // todo: connected状態でないとlog_storeにログが記録されない
            (*log_s)->push_back(*log);
        }
        if (!log_send->empty()) {
            Message::pack(buffer, len, Message::Log{{}, 0, log_send});
        }

        data->message_queue.push(Message::packDone(buffer, len));
    }
    while (auto func_sync = data->func_sync_queue.pop()) {
        (*func_sync)->sync();
    }
}
void Client::onRecv(const std::string &message) {
    namespace MessageKind = WebCFace::Message::MessageKind;
    auto messages = WebCFace::Message::unpack(message, data->logger_internal);
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::sync: {
            auto r = std::any_cast<WebCFace::Message::Sync>(obj);
            auto member = data->getMemberNameFromId(r.member_id);
            data->sync_time_store.setRecv(member, r.getTime());
            data->sync_event.dispatch(member, Field{data, member});
            break;
        }
        case MessageKind::value + MessageKind::res: {
            auto r =
                std::any_cast<WebCFace::Message::Res<WebCFace::Message::Value>>(
                    obj);
            auto [member, field] =
                data->value_store.getReq(r.req_id, r.sub_field);
            data->value_store.setRecv(
                member, field,
                std::static_pointer_cast<VectorOpt<double>>(r.data));
            data->value_change_event.dispatch(FieldBase{member, field},
                                              Field{data, member, field});
            break;
        }
        case MessageKind::text + MessageKind::res: {
            auto r =
                std::any_cast<WebCFace::Message::Res<WebCFace::Message::Text>>(
                    obj);
            auto [member, field] =
                data->text_store.getReq(r.req_id, r.sub_field);
            data->text_store.setRecv(member, field, r.data);
            data->text_change_event.dispatch(FieldBase{member, field},
                                             Field{data, member, field});
            break;
        }
        case MessageKind::view + MessageKind::res: {
            auto r =
                std::any_cast<WebCFace::Message::Res<WebCFace::Message::View>>(
                    obj);
            auto [member, field] =
                data->view_store.getReq(r.req_id, r.sub_field);
            auto v_prev = data->view_store.getRecv(member, field);
            if (v_prev == std::nullopt) {
                v_prev =
                    std::make_shared<std::vector<ViewComponentBase>>(r.length);
                data->view_store.setRecv(member, field, *v_prev);
            }
            (*v_prev)->resize(r.length);
            for (const auto &d : *r.data_diff) {
                (**v_prev)[d.first] = d.second;
            }
            data->view_change_event.dispatch(FieldBase{member, field},
                                             Field{data, member, field});
            break;
        }
        case MessageKind::log: {
            auto r = std::any_cast<WebCFace::Message::Log>(obj);
            auto member = data->getMemberNameFromId(r.member_id);
            auto log_s = data->log_store.getRecv(member);
            if (!log_s) {
                log_s = std::make_shared<std::vector<LogLine>>();
                data->log_store.setRecv(member, *log_s);
            }
            for (const auto &lm : *r.log) {
                (*log_s)->push_back(lm);
            }
            data->log_append_event.dispatch(member, Field{data, member});
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
                            {}, r.caller_id, r.caller_member_id, true}));
                    ValAdaptor result;
                    bool is_error = false;
                    try {
                        result = (*func_info)->run(r.args);
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
                        WebCFace::Message::CallResult{{},
                                                      r.caller_id,
                                                      r.caller_member_id,
                                                      is_error,
                                                      result}));
                } else {

                    data->message_queue.push(WebCFace::Message::packSingle(
                        WebCFace::Message::CallResponse{
                            {}, r.caller_id, r.caller_member_id, false}));
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
            data->value_store.setEntry(r.member_name);
            data->text_store.setEntry(r.member_name);
            data->func_store.setEntry(r.member_name);
            data->member_ids[r.member_name] = r.member_id;
            data->member_entry_event.dispatch(0, Field{data, r.member_name});
            break;
        }
        case MessageKind::entry + MessageKind::value: {
            auto r = std::any_cast<
                WebCFace::Message::Entry<WebCFace::Message::Value>>(obj);
            auto member = data->getMemberNameFromId(r.member_id);
            data->value_store.setEntry(member, r.field);
            data->value_entry_event.dispatch(member,
                                             Field{data, member, r.field});
            break;
        }
        case MessageKind::entry + MessageKind::text: {
            auto r = std::any_cast<
                WebCFace::Message::Entry<WebCFace::Message::Text>>(obj);
            auto member = data->getMemberNameFromId(r.member_id);
            data->text_store.setEntry(member, r.field);
            data->text_entry_event.dispatch(member,
                                            Field{data, member, r.field});
            break;
        }
        case MessageKind::entry + MessageKind::view: {
            auto r = std::any_cast<
                WebCFace::Message::Entry<WebCFace::Message::View>>(obj);
            auto member = data->getMemberNameFromId(r.member_id);
            data->view_store.setEntry(member, r.field);
            data->view_entry_event.dispatch(member,
                                            Field{data, member, r.field});
            break;
        }
        case MessageKind::func_info: {
            auto r = std::any_cast<WebCFace::Message::FuncInfo>(obj);
            auto member = data->getMemberNameFromId(r.member_id);
            data->func_store.setEntry(member, r.field);
            data->func_store.setRecv(member, r.field,
                                     std::make_shared<FuncInfo>(r));
            data->func_entry_event.dispatch(member,
                                            Field{data, member, r.field});
            break;
        }
        case MessageKind::value:
        case MessageKind::text:
        case MessageKind::view:
            this->data->logger_internal->warn("Invalid Message Kind {}", kind);
            break;
        case MessageKind::unknown:
            break;
        default:
            this->data->logger_internal->warn("Unknown Message Kind {}", kind);
            break;
        }
    }
}
} // namespace WebCFace
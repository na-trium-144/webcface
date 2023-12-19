#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/common/def.h>
#include <string>
#include <chrono>
#include "../message/message.h"
#include "client_internal.h"

namespace webcface {

Client::Client(const std::string &name, const std::string &host, int port)
    : Client(name, std::make_shared<Internal::ClientData>(name, host, port)) {}

Client::Client(const std::string &name,
               std::shared_ptr<Internal::ClientData> data)
    : Member(data, name), data(data) {}

Internal::ClientData::ClientData(const std::string &name,
                                 const std::string &host, int port)
    : std::enable_shared_from_this<ClientData>(), self_member_name(name),
      host(host), port(port),
      message_queue(std::make_shared<Common::Queue<std::string>>()),
      value_store(name), text_store(name), func_store(name), view_store(name),
      log_store(std::make_shared<
                SyncDataStore1<std::shared_ptr<std::vector<LogLine>>>>(name)),
      sync_time_store(name),

      logger_sink(std::make_shared<LoggerSink>(log_store)) {
    static auto stderr_sink =
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks = {logger_sink, stderr_sink};
    logger = std::make_shared<spdlog::logger>(name, sinks.begin(), sinks.end());
    logger->set_level(spdlog::level::trace);
    logger_internal = std::make_shared<spdlog::logger>(
        "webcface_internal(" + name + ")", stderr_sink);
    if (std::getenv("WEBCFACE_TRACE") != nullptr) {
        logger_internal->set_level(spdlog::level::trace);
    } else if (getenv("WEBCFACE_VERBOSE") != nullptr) {
        logger_internal->set_level(spdlog::level::debug);
    } else {
        logger_internal->set_level(spdlog::level::off);
    }
    logger_buf = std::make_unique<LoggerBuf>(logger);
    logger_os = std::make_unique<std::ostream>(logger_buf.get());
    log_store->setRecv(name, std::make_shared<std::vector<LogLine>>());
    syncDataFirst();
}
void Internal::ClientData::start() {
    if (message_thread == nullptr) {
        message_thread = std::make_unique<std::thread>(
            Internal::messageThreadMain, shared_from_this(), host, port);
    }
    if (recv_thread == nullptr) {
        recv_thread = std::make_unique<std::thread>(Internal::recvThreadMain,
                                                    shared_from_this());
    }
}


Client::~Client() {
    close();
    data->join();
}
void Client::close() { data->closing.store(true); }
bool Client::connected() const { return data->connected.load(); }
void Internal::ClientData::join() {
    if (message_thread != nullptr && message_thread->joinable()) {
        message_thread->join();
    }
    if (recv_thread != nullptr && recv_thread->joinable()) {
        recv_thread->join();
    }
}
std::vector<Member> Client::members() {
    auto keys = data->value_store.getMembers();
    std::vector<Member> ret(keys.size());
    for (std::size_t i = 0; i < keys.size(); i++) {
        ret[i] = member(keys[i]);
    }
    return ret;
}
EventTarget<Member, int> Client::onMemberEntry() {
    return EventTarget<Member, int>{&data->member_entry_event, 0};
}
void Client::setDefaultRunCond(FuncWrapperType wrapper) {
    data->default_func_wrapper = wrapper;
}
std::shared_ptr<LoggerSink> Client::loggerSink() { return data->logger_sink; }
std::shared_ptr<spdlog::logger> Client::logger() { return data->logger; }
LoggerBuf *Client::loggerStreamBuf() { return data->logger_buf.get(); }
std::ostream &Client::loggerOStream() { return *data->logger_os.get(); }
std::string Client::serverVersion() const { return data->svr_version; }
std::string Client::serverName() const { return data->svr_name; }

void Internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        message_queue->push(Message::packSingle(Message::PingStatusReq{}));
    }
    ping_status_req = true;
}

void Internal::recvThreadMain(std::shared_ptr<ClientData> data) {
    while (!data->closing.load()) {
        auto msg = data->recv_queue.pop(std::chrono::milliseconds(10));
        if (msg) {
            data->onRecv(*msg);
        }
    }
}

void Client::start() {
    if (!connected()) {
        data->start();
    }
}
void Client::waitConnection() {
    if (!connected()) {
        data->start();
        std::unique_lock lock(data->connect_state_m);
        data->connect_state_cond.wait(lock, [this] { return connected(); });
    }
}
void Internal::ClientData::syncDataFirst() {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard log_lock(log_store->mtx);

    std::stringstream buffer;
    int len = 0;

    Message::pack(buffer, len,
                  Message::SyncInit{
                      {}, self_member_name, 0, "cpp", WEBCFACE_VERSION, ""});

    for (const auto &v : value_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(
                buffer, len,
                Message::Req<Message::Value>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : text_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(
                buffer, len,
                Message::Req<Message::Text>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : view_store.transferReq()) {
        for (const auto &v2 : v.second) {
            Message::pack(
                buffer, len,
                Message::Req<Message::View>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : log_store->transferReq()) {
        Message::pack(buffer, len, Message::LogReq{{}, v.first});
    }

    syncData(true);

    if (ping_status_req) {
        Message::pack(buffer, len, Message::PingStatusReq{});
    }

    message_queue->push(Message::packDone(buffer, len));
}
void Internal::ClientData::syncData(bool is_first) {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard log_lock(log_store->mtx);

    std::stringstream buffer;
    int len = 0;

    Message::pack(buffer, len, Message::Sync{});

    for (const auto &v : value_store.transferSend(is_first)) {
        Message::pack(
            buffer, len,
            Message::Value{
                {},
                v.first,
                std::static_pointer_cast<std::vector<double>>(v.second)});
    }
    for (const auto &v : text_store.transferSend(is_first)) {
        Message::pack(buffer, len, Message::Text{{}, v.first, v.second});
    }
    auto view_send_prev = view_store.getSendPrev(is_first);
    auto view_send = view_store.transferSend(is_first);
    for (const auto &v : view_send) {
        auto v_prev = view_send_prev.find(v.first);
        auto v_diff =
            std::make_shared<std::unordered_map<int, ViewComponentBase>>();
        if (v_prev == view_send_prev.end()) {
            for (std::size_t i = 0; i < v.second->size(); i++) {
                v_diff->emplace(static_cast<int>(i), (*v.second)[i]);
            }
        } else {
            for (std::size_t i = 0; i < v.second->size(); i++) {
                if (v_prev->second->size() <= i ||
                    (*v_prev->second)[i] != (*v.second)[i]) {
                    v_diff->emplace(static_cast<int>(i), (*v.second)[i]);
                }
            }
        }
        if (!v_diff->empty()) {
            Message::pack(buffer, len,
                          Message::View{v.first, v_diff, v.second->size()});
        }
    }

    auto log_s = *log_store->getRecv(self_member_name);
    if ((log_s->size() > 0 && is_first) || log_s->size() > log_sent_lines) {
        auto begin = log_s->begin();
        auto end = log_s->end();
        if (!is_first) {
            begin += log_sent_lines;
        }
        log_sent_lines = log_s->size();
        Message::pack(buffer, len, Message::Log{begin, end});
    }
    for (const auto &v : func_store.transferSend(is_first)) {
        if (!v.second->hidden) {
            Message::pack(buffer, len, Message::FuncInfo{v.first, *v.second});
        }
    }

    message_queue->push(Message::packDone(buffer, len));
}
void Client::sync() {
    start();
    data->syncData(false);
    while (auto func_sync = data->func_sync_queue.pop()) {
        (*func_sync)->sync();
    }
}
void Internal::ClientData::onRecv(const std::string &message) {
    namespace MessageKind = webcface::Message::MessageKind;
    auto messages = webcface::Message::unpack(message, this->logger_internal);
    std::vector<std::string> sync_members;
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::svr_version: {
            auto r = std::any_cast<webcface::Message::SvrVersion>(obj);
            this->svr_name = r.svr_name;
            this->svr_version = r.ver;
            break;
        }
        case MessageKind::ping: {
            this->message_queue->push(
                webcface::Message::packSingle(webcface::Message::Ping{}));
            break;
        }
        case MessageKind::ping_status: {
            auto r = std::any_cast<webcface::Message::PingStatus>(obj);
            this->ping_status = r.status;
            for (const auto &member_name : value_store.getMembers()) {
                this->ping_event.dispatch(
                    member_name, Field{shared_from_this(), member_name});
            }
            break;
        }
        case MessageKind::sync: {
            auto r = std::any_cast<webcface::Message::Sync>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->sync_time_store.setRecv(member, r.getTime());
            sync_members.push_back(member);
            break;
        }
        case MessageKind::value + MessageKind::res: {
            auto r =
                std::any_cast<webcface::Message::Res<webcface::Message::Value>>(
                    obj);
            auto [member, field] =
                this->value_store.getReq(r.req_id, r.sub_field);
            this->value_store.setRecv(
                member, field,
                std::static_pointer_cast<VectorOpt<double>>(r.data));
            this->value_change_event.dispatch(
                FieldBase{member, field},
                Field{shared_from_this(), member, field});
            break;
        }
        case MessageKind::text + MessageKind::res: {
            auto r =
                std::any_cast<webcface::Message::Res<webcface::Message::Text>>(
                    obj);
            auto [member, field] =
                this->text_store.getReq(r.req_id, r.sub_field);
            this->text_store.setRecv(member, field, r.data);
            this->text_change_event.dispatch(
                FieldBase{member, field},
                Field{shared_from_this(), member, field});
            break;
        }
        case MessageKind::view + MessageKind::res: {
            auto r =
                std::any_cast<webcface::Message::Res<webcface::Message::View>>(
                    obj);
            std::lock_guard lock(this->view_store.mtx);
            auto [member, field] =
                this->view_store.getReq(r.req_id, r.sub_field);
            auto v_prev = this->view_store.getRecv(member, field);
            if (v_prev == std::nullopt) {
                v_prev =
                    std::make_shared<std::vector<ViewComponentBase>>(r.length);
                this->view_store.setRecv(member, field, *v_prev);
            }
            (*v_prev)->resize(r.length);
            for (const auto &d : *r.data_diff) {
                (**v_prev)[std::stoi(d.first)] = d.second;
            }
            this->view_change_event.dispatch(
                FieldBase{member, field},
                Field{shared_from_this(), member, field});
            break;
        }
        case MessageKind::log: {
            auto r = std::any_cast<webcface::Message::Log>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            std::lock_guard lock(this->log_store->mtx);
            auto log_s = this->log_store->getRecv(member);
            if (!log_s) {
                log_s = std::make_shared<std::vector<LogLine>>();
                this->log_store->setRecv(member, *log_s);
            }
            for (const auto &lm : *r.log) {
                (*log_s)->push_back(lm);
            }
            this->log_append_event.dispatch(member,
                                            Field{shared_from_this(), member});
            break;
        }
        case MessageKind::call: {
            auto r = std::any_cast<webcface::Message::Call>(obj);
            std::thread([data = shared_from_this(), r] {
                auto func_info =
                    data->func_store.getRecv(data->self_member_name, r.field);
                if (func_info) {
                    data->message_queue->push(webcface::Message::packSingle(
                        webcface::Message::CallResponse{
                            {}, r.caller_id, r.caller_member_id, true}));
                    ValAdaptor result;
                    bool is_error = false;
                    try {
                        result = (*func_info)->run(r.args);
                    } catch (const std::exception &e) {
                        is_error = true;
                        result = std::string(e.what());
                    } catch (const std::string &e) {
                        is_error = true;
                        result = e;
                    } catch (...) {
                        is_error = true;
                        result = "unknown exception";
                    }
                    data->message_queue->push(webcface::Message::packSingle(
                        webcface::Message::CallResult{{},
                                                      r.caller_id,
                                                      r.caller_member_id,
                                                      is_error,
                                                      result}));
                } else {
                    data->message_queue->push(webcface::Message::packSingle(
                        webcface::Message::CallResponse{
                            {}, r.caller_id, r.caller_member_id, false}));
                }
            }).detach();
            break;
        }
        case MessageKind::call_response: {
            auto r = std::any_cast<webcface::Message::CallResponse>(obj);
            try {
                auto &res = this->func_result_store.getResult(r.caller_id);
                res.started_->set_value(r.started);
                if (!r.started) {
                    try {
                        throw FuncNotFound(res);
                    } catch (...) {
                        res.result_->set_exception(std::current_exception());
                    }
                }
            } catch (const std::future_error &e) {
                this->logger_internal->error(
                    "error receiving call response id={}: {}", r.caller_id,
                    e.what());
            } catch (const std::out_of_range &e) {
                this->logger_internal->error(
                    "error receiving call response id={}: {}", r.caller_id,
                    e.what());
            }
            break;
        }
        case MessageKind::call_result: {
            auto r = std::any_cast<webcface::Message::CallResult>(obj);
            try {
                auto &res = this->func_result_store.getResult(r.caller_id);
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
            } catch (const std::future_error &e) {
                this->logger_internal->error(
                    "error receiving call result id={}: {}", r.caller_id,
                    e.what());
            } catch (const std::out_of_range &e) {
                this->logger_internal->error(
                    "error receiving call response id={}: {}", r.caller_id,
                    e.what());
            }
            break;
        }
        case MessageKind::sync_init: {
            auto r = std::any_cast<webcface::Message::SyncInit>(obj);
            this->value_store.setEntry(r.member_name);
            this->text_store.setEntry(r.member_name);
            this->func_store.setEntry(r.member_name);
            this->member_ids[r.member_name] = r.member_id;
            this->member_lib_name[r.member_id] = r.lib_name;
            this->member_lib_ver[r.member_id] = r.lib_ver;
            this->member_addr[r.member_id] = r.addr;
            this->member_entry_event.dispatch(
                0, Field{shared_from_this(), r.member_name});
            break;
        }
        case MessageKind::entry + MessageKind::value: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::Value>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->value_store.setEntry(member, r.field);
            this->value_entry_event.dispatch(
                member, Field{shared_from_this(), member, r.field});
            break;
        }
        case MessageKind::entry + MessageKind::text: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::Text>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->text_store.setEntry(member, r.field);
            this->text_entry_event.dispatch(
                member, Field{shared_from_this(), member, r.field});
            break;
        }
        case MessageKind::entry + MessageKind::view: {
            auto r = std::any_cast<
                webcface::Message::Entry<webcface::Message::View>>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->view_store.setEntry(member, r.field);
            this->view_entry_event.dispatch(
                member, Field{shared_from_this(), member, r.field});
            break;
        }
        case MessageKind::func_info: {
            auto r = std::any_cast<webcface::Message::FuncInfo>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->func_store.setEntry(member, r.field);
            this->func_store.setRecv(member, r.field,
                                     std::make_shared<FuncInfo>(r));
            this->func_entry_event.dispatch(
                member, Field{shared_from_this(), member, r.field});
            break;
        }
        case MessageKind::value:
        case MessageKind::text:
        case MessageKind::view:
        case MessageKind::value + MessageKind::req:
        case MessageKind::text + MessageKind::req:
        case MessageKind::view + MessageKind::req:
        case MessageKind::ping_status_req:
        case MessageKind::log_req:
            this->logger_internal->warn("Invalid Message Kind {}", kind);
            break;
        case MessageKind::unknown:
            break;
        default:
            this->logger_internal->warn("Unknown Message Kind {}", kind);
            break;
        }
    }
    for (const auto &m : sync_members) {
        this->sync_event.dispatch(m, Field{shared_from_this(), m});
    }
}
} // namespace webcface

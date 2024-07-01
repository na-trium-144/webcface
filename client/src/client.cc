#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/image.h>
#include <webcface/robot_model.h>
#include <webcface/canvas3d.h>
#include <webcface/canvas2d.h>
#include <webcface/common/def.h>
#include "webcface/message/message.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/client_ws.h"
#include <string>
#include <chrono>

WEBCFACE_NS_BEGIN

Client::Client(const SharedString &name, const SharedString &host, int port)
    : Client(name, std::make_shared<internal::ClientData>(name, host, port)) {}

Client::Client(const SharedString &name,
               const std::shared_ptr<internal::ClientData> &data)
    : Member(data, name), data(data) {}

internal::ClientData::ClientData(const SharedString &name,
                                 const SharedString &host, int port)
    : std::enable_shared_from_this<ClientData>(), self_member_name(name),
      host(host), port(port), current_curl_handle(nullptr),
      current_curl_closed(false), current_curl_path(), current_ws_buf(),
      message_queue(std::make_shared<Queue<std::string>>()), value_store(name),
      text_store(name), func_store(name), view_store(name), image_store(name),
      robot_model_store(name), canvas3d_store(name), canvas2d_store(name),
      log_store(std::make_shared<
                SyncDataStore1<std::shared_ptr<std::vector<LogLineData<>>>>>(
          name)),
      sync_time_store(name),
      logger_sink(std::make_shared<LoggerSink>(log_store)) {
    static auto stderr_sink =
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    std::vector<spdlog::sink_ptr> sinks = {logger_sink, stderr_sink};
    logger = std::make_shared<spdlog::logger>(name.decode(), sinks.begin(),
                                              sinks.end());
    logger->set_level(spdlog::level::trace);
    logger_internal = std::make_shared<spdlog::logger>(
        "webcface_internal(" + name.decode() + ")", stderr_sink);
    if (std::getenv("WEBCFACE_TRACE") != nullptr) {
        logger_internal->set_level(spdlog::level::trace);
    } else if (std::getenv("WEBCFACE_VERBOSE") != nullptr) {
        logger_internal->set_level(spdlog::level::debug);
    } else {
        logger_internal->set_level(spdlog::level::off);
    }
    logger_buf = std::make_unique<LoggerBuf>(log_store);
    logger_os = std::make_unique<std::ostream>(logger_buf.get());
    logger_buf_w = std::make_unique<LoggerBufW>(log_store);
    logger_os_w = std::make_unique<std::wostream>(logger_buf_w.get());
    log_store->setRecv(name, std::make_shared<std::vector<LogLineData<>>>());
}
void internal::ClientData::start() {
    if (!message_thread) {
        message_thread = std::make_unique<std::thread>(
            internal::messageThreadMain, shared_from_this());
    }
    if (recv_thread && !recv_thread_running.load()) {
        recv_thread->join();
        recv_thread = nullptr;
    }
    if (!recv_thread) {
        recv_thread_running.store(true);
        recv_thread = std::make_unique<std::thread>(internal::recvThreadMain,
                                                    shared_from_this());
    }
}

Client::~Client() {
    close();
    data->join();
}
void Client::close() { data->closing.store(true); }
bool Client::connected() const { return data->connected.load(); }
void internal::ClientData::join() {
    if (message_thread && message_thread->joinable()) {
        message_thread->join();
    }
    if (recv_thread && recv_thread->joinable()) {
        recv_thread->join();
    }
}
std::vector<Member> Client::members() {
    std::lock_guard lock(data->entry_m);
    std::vector<Member> ret;
    ret.reserve(data->member_entry.size());
    for (const auto &m : data->member_entry) {
        ret.push_back(member(m));
    }
    return ret;
}
EventTarget<Member> Client::onMemberEntry() {
    std::lock_guard lock(data->event_m);
    return EventTarget<Member>{data->member_entry_event};
}
void Client::setDefaultRunCond(const FuncWrapperType &wrapper) {
    data->default_func_wrapper = wrapper;
}
std::shared_ptr<LoggerSink> Client::loggerSink() { return data->logger_sink; }
std::shared_ptr<spdlog::logger> Client::logger() { return data->logger; }
LoggerBuf *Client::loggerStreamBuf() { return data->logger_buf.get(); }
std::ostream &Client::loggerOStream() { return *data->logger_os.get(); }
std::string Client::serverVersion() const { return data->svr_version; }
std::string Client::serverName() const { return data->svr_name; }

void internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        message_queue->push(message::packSingle(message::PingStatusReq{}));
    }
    ping_status_req = true;
}

void internal::recvThreadMain(const std::shared_ptr<ClientData> &data) {
    while (!data->closing.load() && data->port > 0) {
        internal::WebSocket::init(data);
        while (data->connected.load() && !data->current_curl_closed &&
               !data->closing.load()) {
            internal::WebSocket::recv(data);
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            // std::this_thread::yield();
        }
        internal::WebSocket::close(data);
        if (!data->auto_reconnect.load()) {
            break;
        }
        if (!data->closing.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    {
        std::lock_guard lock(data->connect_state_m);
        data->recv_thread_running.store(false);
        data->connect_state_cond.notify_all();
    }
}
void internal::messageThreadMain(
    const std::shared_ptr<internal::ClientData> &data) {
    while (!data->closing.load()) {
        auto msg = data->message_queue->pop(std::chrono::milliseconds(10));
        while (msg && !data->closing.load()) {
            std::unique_lock lock(data->connect_state_m);
            data->connect_state_cond.wait_for(
                lock, std::chrono::milliseconds(10),
                [&] { return data->connected.load() || data->closing.load(); });
            if (data->connected.load()) {
                internal::WebSocket::send(data, *msg);
                break;
            }
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
        data->connect_state_cond.wait(lock, [this] {
            return connected() || !data->recv_thread_running.load();
        });
    }
}
void Client::autoReconnect(bool enabled) {
    data->auto_reconnect.store(enabled);
}
bool Client::autoReconnect() const { return data->auto_reconnect.load(); }

std::string internal::ClientData::syncDataFirst() {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard image_lock(image_store.mtx);
    std::lock_guard robot_model_lock(robot_model_store.mtx);
    std::lock_guard canvas3d_lock(canvas3d_store.mtx);
    std::lock_guard canvas2d_lock(canvas2d_store.mtx);
    std::lock_guard log_lock(log_store->mtx);

    std::stringstream buffer;
    int len = 0;

    message::pack(buffer, len,
                  message::SyncInit{
                      {}, self_member_name, 0, "cpp", WEBCFACE_VERSION, ""});

    for (const auto &v : value_store.transferReq()) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::Value>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : text_store.transferReq()) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::Text>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : view_store.transferReq()) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::View>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : robot_model_store.transferReq()) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::RobotModel>{
                              {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : canvas3d_store.transferReq()) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Canvas3D>{
                              {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : canvas2d_store.transferReq()) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Canvas2D>{
                              {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : image_store.transferReq()) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Image>{
                              v.first, v2.first, v2.second,
                              image_store.getReqInfo(v.first, v2.first)});
        }
    }
    for (const auto &v : log_store->transferReq()) {
        message::pack(buffer, len, message::LogReq{{}, v.first});
    }

    if (ping_status_req) {
        message::pack(buffer, len, message::PingStatusReq{});
    }

    return syncData(true, buffer, len);
}
std::string internal::ClientData::syncData(bool is_first) {
    std::stringstream buffer;
    int len = 0;
    return syncData(is_first, buffer, len);
}
std::string internal::ClientData::syncData(bool is_first,
                                           std::stringstream &buffer,
                                           int &len) {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard image_lock(image_store.mtx);
    std::lock_guard robot_model_lock(robot_model_store.mtx);
    std::lock_guard canvas3d_lock(canvas3d_store.mtx);
    std::lock_guard canvas2d_lock(canvas2d_store.mtx);
    std::lock_guard log_lock(log_store->mtx);

    message::pack(buffer, len, message::Sync{});

    for (const auto &v : value_store.transferSend(is_first)) {
        message::pack(
            buffer, len,
            message::Value{
                {},
                v.first,
                std::static_pointer_cast<std::vector<double>>(v.second)});
    }
    for (const auto &v : text_store.transferSend(is_first)) {
        message::pack(buffer, len, message::Text{{}, v.first, v.second});
    }
    for (const auto &v : robot_model_store.transferSend(is_first)) {
        auto data = std::make_shared<std::vector<message::RobotLink>>();
        data->reserve(v.second->size());
        std::vector<SharedString> link_names;
        link_names.reserve(v.second->size());
        for (std::size_t i = 0; i < v.second->size(); i++) {
            data->emplace_back(v.second->at(i).toMessage(link_names));
            link_names.push_back((*data)[i].name);
        }
        message::pack(buffer, len, message::RobotModel{v.first, data});
    }
    auto view_prev = view_store.getSendPrev(is_first);
    for (const auto &p : view_store.transferSend(is_first)) {
        auto v_prev = view_prev.find(p.first);
        auto v_diff = view_store.getDiff(
            p.second.get(),
            v_prev == view_prev.end() ? nullptr : v_prev->second.get());
        if (!v_diff->empty()) {
            message::pack(buffer, len,
                          message::View{p.first, v_diff, p.second->size()});
        }
    }
    auto canvas3d_prev = canvas3d_store.getSendPrev(is_first);
    for (const auto &p : canvas3d_store.transferSend(is_first)) {
        auto v_prev = canvas3d_prev.find(p.first);
        auto v_diff = canvas3d_store.getDiff(
            p.second.get(),
            v_prev == canvas3d_prev.end() ? nullptr : v_prev->second.get());
        if (!v_diff->empty()) {
            message::pack(buffer, len,
                          message::Canvas3D{p.first, v_diff, p.second->size()});
        }
    }
    auto canvas2d_prev = canvas2d_store.getSendPrev(is_first);
    for (const auto &p : canvas2d_store.transferSend(is_first)) {
        auto v_prev = canvas2d_prev.find(p.first);
        auto v_diff = view_store.getDiff(&p.second->components,
                                         v_prev == canvas2d_prev.end()
                                             ? nullptr
                                             : &v_prev->second->components);
        if (!v_diff->empty()) {
            message::pack(buffer, len,
                          message::Canvas2D{p.first, p.second->width,
                                            p.second->height, v_diff,
                                            p.second->components.size()});
        }
    }
    for (const auto &v : image_store.transferSend(is_first)) {
        message::pack(buffer, len,
                      message::Image{v.first, v.second.toMessage()});
    }

    if (log_store) {
        auto log_self_opt = log_store->getRecv(self_member_name);
        if (!log_self_opt) [[unlikely]] {
            throw std::runtime_error("self log data is null");
        } else {
            auto &log_s = *log_self_opt;
            if ((log_s->size() > 0 && is_first) ||
                log_s->size() > log_sent_lines) {
                auto begin = log_s->begin();
                auto end = log_s->end();
                if (!is_first) {
                    begin += static_cast<int>(log_sent_lines);
                }
                log_sent_lines = log_s->size();
                message::pack(buffer, len, message::Log{begin, end});
            }
        }
    }
    for (const auto &v : func_store.transferSend(is_first)) {
        if (!v.first.u8String().starts_with(field_separator)) {
            message::pack(buffer, len, v.second->toMessage(v.first));
        }
    }

    return message::packDone(buffer, len);
}
void Client::sync() {
    start();
    data->message_queue->push(data->syncData(false));
    while (auto func_sync = data->func_sync_queue.pop()) {
        (*func_sync)->sync();
    }
}

template <typename M, typename K1, typename K2>
static auto findFromMap2(const M &map, const K1 &key1, const K2 &key2)
    -> std::optional<std::remove_cvref_t<decltype(map.at(key1).at(key2))>> {
    auto s_it = map.find(key1);
    if (s_it != map.end()) {
        auto it = s_it->second.find(key2);
        if (it != s_it->second.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}
template <typename M, typename K1>
static auto findFromMap1(const M &map, const K1 &key1)
    -> std::optional<std::remove_cvref_t<decltype(map.at(key1))>> {
    auto it = map.find(key1);
    if (it != map.end()) {
        return it->second;
    }
    return std::nullopt;
}
template <typename Msg, typename T, typename S, typename E>
static void onRecvRes(internal::ClientData *this_, const Msg &r, const T &data,
                      S &store, const E &event) {
    auto [member, field] = store.getReq(r.req_id, r.sub_field);
    store.setRecv(member, field, data);
    std::remove_cvref_t<decltype(event.at(member).at(field))> cl;
    {
        std::lock_guard lock(this_->event_m);
        cl = findFromMap2(event, member, field).value_or(nullptr);
    }
    if (cl) {
        cl->operator()(Field{this_->shared_from_this(), member, field});
    }
}
template <typename Msg, typename S, typename E>
static void onRecvEntry(internal::ClientData *this_, const Msg &r, S &store,
                        const E &event) {
    auto member = this_->getMemberNameFromId(r.member_id);
    store.setEntry(member, r.field);
    std::remove_cvref_t<decltype(event.at(member))> cl;
    {
        std::lock_guard lock(this_->event_m);
        cl = findFromMap1(event, member).value_or(nullptr);
    }
    if (cl) {
        cl->operator()(Field{this_->shared_from_this(), member, r.field});
    }
}

void internal::ClientData::onRecv(const std::string &message) {
    static std::unordered_map<int, bool> message_kind_warned;
    namespace MessageKind = webcface::message::MessageKind;
    auto messages = webcface::message::unpack(message, this->logger_internal);
    std::vector<SharedString> sync_members;
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::svr_version: {
            auto r = std::any_cast<webcface::message::SvrVersion>(obj);
            this->svr_name = r.svr_name;
            this->svr_version = r.ver;
            break;
        }
        case MessageKind::ping: {
            this->message_queue->push(
                webcface::message::packSingle(webcface::message::Ping{}));
            break;
        }
        case MessageKind::ping_status: {
            auto r = std::any_cast<webcface::message::PingStatus>(obj);
            this->ping_status = r.status;
            StrSet1 members;
            {
                std::lock_guard lock(entry_m);
                members = this->member_entry;
            }
            for (const auto &member_name : members) {
                std::shared_ptr<eventpp::CallbackList<void(Member)>> cl;
                {
                    std::lock_guard lock(event_m);
                    cl = findFromMap1(this->ping_event, member_name)
                             .value_or(nullptr);
                }
                if (cl) {
                    cl->operator()(Field{shared_from_this(), member_name});
                }
            }
            break;
        }
        case MessageKind::sync: {
            auto r = std::any_cast<webcface::message::Sync>(obj);
            const auto &member = this->getMemberNameFromId(r.member_id);
            this->sync_time_store.setRecv(member, r.getTime());
            sync_members.push_back(member);
            break;
        }
        case MessageKind::value + MessageKind::res: {
            auto r =
                std::any_cast<webcface::message::Res<webcface::message::Value>>(
                    obj);
            onRecvRes(this, r, r.data, this->value_store,
                      this->value_change_event);
            break;
        }
        case MessageKind::text + MessageKind::res: {
            auto r =
                std::any_cast<webcface::message::Res<webcface::message::Text>>(
                    obj);
            onRecvRes(this, r, r.data, this->text_store,
                      this->text_change_event);
            break;
        }
        case MessageKind::robot_model + MessageKind::res: {
            auto r = std::any_cast<message::Res<message::RobotModel>>(obj);
            auto common_links = std::make_shared<std::vector<RobotLink>>();
            common_links->reserve(r.data->size());
            std::vector<SharedString> link_names;
            link_names.reserve(r.data->size());
            for (std::size_t i = 0; i < r.data->size(); i++) {
                common_links->emplace_back((*r.data)[i], link_names);
                link_names.push_back((*r.data)[i].name);
            }
            onRecvRes(this, r, common_links, this->robot_model_store,
                      this->robot_model_change_event);
            break;
        }
        case MessageKind::view + MessageKind::res: {
            auto r =
                std::any_cast<webcface::message::Res<webcface::message::View>>(
                    obj);
            std::lock_guard lock(this->view_store.mtx);
            auto [member, field] =
                this->view_store.getReq(r.req_id, r.sub_field);
            auto v_prev = this->view_store.getRecv(member, field);
            std::shared_ptr<std::vector<ViewComponent>> vv_prev;
            if (v_prev) {
                vv_prev = *v_prev;
            } else {
                vv_prev =
                    std::make_shared<std::vector<ViewComponent>>(r.length);
                v_prev.emplace(vv_prev);
                this->view_store.setRecv(member, field, vv_prev);
            }
            vv_prev->resize(r.length);
            for (const auto &d : *r.data_diff) {
                (*vv_prev)[std::stoi(d.first)] = d.second;
            }
            std::shared_ptr<eventpp::CallbackList<void(View)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap2(this->view_change_event, member, field)
                         .value_or(nullptr);
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::canvas3d + MessageKind::res: {
            auto r = std::any_cast<
                webcface::message::Res<webcface::message::Canvas3D>>(obj);
            std::lock_guard lock(this->canvas3d_store.mtx);
            auto [member, field] =
                this->canvas3d_store.getReq(r.req_id, r.sub_field);
            auto v_prev = this->canvas3d_store.getRecv(member, field);
            std::shared_ptr<std::vector<Canvas3DComponent>> vv_prev;
            if (v_prev) {
                vv_prev = *v_prev;
            } else {
                vv_prev =
                    std::make_shared<std::vector<Canvas3DComponent>>(r.length);
                v_prev.emplace(vv_prev);
                this->canvas3d_store.setRecv(member, field, vv_prev);
            }
            vv_prev->resize(r.length);
            for (const auto &d : *r.data_diff) {
                (*vv_prev)[std::stoi(d.first)] = d.second;
            }
            std::shared_ptr<eventpp::CallbackList<void(Canvas3D)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap2(this->canvas3d_change_event, member, field)
                         .value_or(nullptr);
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::canvas2d + MessageKind::res: {
            auto r = std::any_cast<
                webcface::message::Res<webcface::message::Canvas2D>>(obj);
            std::lock_guard lock(this->canvas2d_store.mtx);
            auto [member, field] =
                this->canvas2d_store.getReq(r.req_id, r.sub_field);
            auto v_prev = this->canvas2d_store.getRecv(member, field);
            std::shared_ptr<Canvas2DDataBase> vv_prev;
            if (v_prev) {
                vv_prev = *v_prev;
            } else {
                vv_prev = std::make_shared<Canvas2DDataBase>();
                v_prev.emplace(vv_prev);
                this->canvas2d_store.setRecv(member, field, vv_prev);
            }
            vv_prev->width = r.width;
            vv_prev->height = r.height;
            vv_prev->components.resize(r.length);
            for (const auto &d : *r.data_diff) {
                vv_prev->components[std::stoi(d.first)] = d.second;
            }
            std::shared_ptr<eventpp::CallbackList<void(Canvas2D)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap2(this->canvas2d_change_event, member, field)
                         .value_or(nullptr);
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, field});
            }
            break;
        }
        case MessageKind::image + MessageKind::res: {
            auto r =
                std::any_cast<webcface::message::Res<webcface::message::Image>>(
                    obj);
            onRecvRes(this, r, r, this->image_store, this->image_change_event);
            break;
        }
        case MessageKind::log: {
            auto r = std::any_cast<webcface::message::Log>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            std::lock_guard lock(this->log_store->mtx);
            auto log_s = this->log_store->getRecv(member);
            if (!log_s) {
                log_s = std::make_shared<std::vector<LogLineData<>>>();
                this->log_store->setRecv(member, *log_s);
            }
            for (auto &lm : *r.log) {
                (*log_s)->emplace_back(lm);
            }
            std::shared_ptr<eventpp::CallbackList<void(Log)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap1(this->log_append_event, member)
                         .value_or(nullptr);
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member});
            }
            break;
        }
        case MessageKind::call: {
            auto r = std::any_cast<webcface::message::Call>(obj);
            std::thread([data = shared_from_this(), r] {
                auto func_info =
                    data->func_store.getRecv(data->self_member_name, r.field);
                if (func_info) {
                    data->message_queue->push(webcface::message::packSingle(
                        webcface::message::CallResponse{
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
                    } catch (const char *e) {
                        is_error = true;
                        result = e;
                    } catch (const std::wstring &e) {
                        is_error = true;
                        result = e;
                    } catch (const wchar_t *e) {
                        is_error = true;
                        result = e;
                    } catch (...) {
                        is_error = true;
                        result = "unknown exception";
                    }
                    data->message_queue->push(webcface::message::packSingle(
                        webcface::message::CallResult{{},
                                                      r.caller_id,
                                                      r.caller_member_id,
                                                      is_error,
                                                      result}));
                } else {
                    data->message_queue->push(webcface::message::packSingle(
                        webcface::message::CallResponse{
                            {}, r.caller_id, r.caller_member_id, false}));
                }
            }).detach();
            break;
        }
        case MessageKind::call_response: {
            auto r = std::any_cast<webcface::message::CallResponse>(obj);
            try {
                this->func_result_store.resultSetter(r.caller_id)
                    .setStarted(r.started);
                if (!r.started) {
                    this->func_result_store.removeResultSetter(r.caller_id);
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
            auto r = std::any_cast<webcface::message::CallResult>(obj);
            try {
                if (r.is_error) {
                    try {
                        throw std::runtime_error(r.result.asStringRef());
                    } catch (...) {
                        this->func_result_store.resultSetter(r.caller_id)
                            .setResultException(std::current_exception());
                    }
                } else {
                    this->func_result_store.resultSetter(r.caller_id)
                        .setResult(r.result);
                    // todo: 戻り値の型?
                }
                this->func_result_store.removeResultSetter(r.caller_id);
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
            auto r = std::any_cast<webcface::message::SyncInit>(obj);
            {
                std::lock_guard lock(this->entry_m);
                this->member_entry.emplace(r.member_name);
            }
            this->value_store.clearEntry(r.member_name);
            this->text_store.clearEntry(r.member_name);
            this->func_store.clearEntry(r.member_name);
            this->view_store.clearEntry(r.member_name);
            this->image_store.clearEntry(r.member_name);
            this->robot_model_store.clearEntry(r.member_name);
            this->canvas3d_store.clearEntry(r.member_name);
            this->canvas2d_store.clearEntry(r.member_name);
            this->member_ids[r.member_name] = r.member_id;
            this->member_lib_name[r.member_id] = r.lib_name;
            this->member_lib_ver[r.member_id] = r.lib_ver;
            this->member_addr[r.member_id] = r.addr;
            if (this->member_entry_event) {
                this->member_entry_event->operator()(
                    Field{shared_from_this(), r.member_name});
            }
            break;
        }
        case MessageKind::entry + MessageKind::value: {
            auto r = std::any_cast<
                webcface::message::Entry<webcface::message::Value>>(obj);
            onRecvEntry(this, r, this->value_store, this->value_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::text: {
            auto r = std::any_cast<
                webcface::message::Entry<webcface::message::Text>>(obj);
            onRecvEntry(this, r, this->text_store, this->text_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::view: {
            auto r = std::any_cast<
                webcface::message::Entry<webcface::message::View>>(obj);
            onRecvEntry(this, r, this->view_store, this->view_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::canvas3d: {
            auto r = std::any_cast<
                webcface::message::Entry<webcface::message::Canvas3D>>(obj);
            onRecvEntry(this, r, this->canvas3d_store,
                        this->canvas3d_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::canvas2d: {
            auto r = std::any_cast<
                webcface::message::Entry<webcface::message::Canvas2D>>(obj);
            onRecvEntry(this, r, this->canvas2d_store,
                        this->canvas2d_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::robot_model: {
            auto r = std::any_cast<
                webcface::message::Entry<webcface::message::RobotModel>>(obj);
            onRecvEntry(this, r, this->robot_model_store,
                        this->robot_model_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::image: {
            auto r = std::any_cast<
                webcface::message::Entry<webcface::message::Image>>(obj);
            onRecvEntry(this, r, this->image_store, this->image_entry_event);
            break;
        }
        case MessageKind::func_info: {
            auto r = std::any_cast<webcface::message::FuncInfo>(obj);
            auto member = this->getMemberNameFromId(r.member_id);
            this->func_store.setEntry(member, r.field);
            this->func_store.setRecv(member, r.field,
                                     std::make_shared<FuncInfo>(r));
            std::shared_ptr<eventpp::CallbackList<void(Func)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap1(this->func_entry_event, member)
                         .value_or(nullptr);
            }
            if (cl) {
                cl->operator()(Field{shared_from_this(), member, r.field});
            }
            break;
        }
        case MessageKind::value:
        case MessageKind::text:
        case MessageKind::view:
        case MessageKind::canvas3d:
        case MessageKind::canvas2d:
        case MessageKind::robot_model:
        case MessageKind::image:
        case MessageKind::value + MessageKind::req:
        case MessageKind::text + MessageKind::req:
        case MessageKind::view + MessageKind::req:
        case MessageKind::canvas3d + MessageKind::req:
        case MessageKind::canvas2d + MessageKind::req:
        case MessageKind::robot_model + MessageKind::req:
        case MessageKind::image + MessageKind::req:
        case MessageKind::ping_status_req:
        case MessageKind::log_req:
            if (!message_kind_warned[kind]) {
                logger->warn("Invalid message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        case MessageKind::unknown:
            break;
        default:
            if (!message_kind_warned[kind]) {
                logger->warn("Unknown message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        }
    }
    for (const auto &m : sync_members) {
        std::shared_ptr<eventpp::CallbackList<void(Member)>> cl;
        {
            std::lock_guard lock(event_m);
            cl = findFromMap1(this->sync_event, m).value_or(nullptr);
        }
        if (cl) {
            cl->operator()(Field{shared_from_this(), m});
        }
    }
}
WEBCFACE_NS_END

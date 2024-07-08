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
#include "webcface/internal/unlock.h"
#include <webcface/internal/logger.h>
#include <string>
#include <chrono>
#include <spdlog/sinks/stdout_color_sinks.h>

WEBCFACE_NS_BEGIN

Client::Client(const SharedString &name, const SharedString &host, int port)
    : Client(name, std::make_shared<internal::ClientData>(name, host, port)) {}

Client::Client(const SharedString &name,
               const std::shared_ptr<internal::ClientData> &data)
    : Member(data, name), data(data) {}

internal::ClientData::ClientData(const SharedString &name,
                                 const SharedString &host, int port)
    : std::enable_shared_from_this<ClientData>(), self_member_name(name),
      host(host), port(port), current_curl_path(), current_ws_buf(),
      value_store(name), text_store(name), func_store(name), view_store(name),
      image_store(name), robot_model_store(name), canvas3d_store(name),
      canvas2d_store(name), log_store(name), sync_time_store(name) {
    static auto stderr_sink =
        std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    logger_internal = std::make_shared<spdlog::logger>(
        "webcface_internal(" + name.decode() + ")", stderr_sink);
    if (std::getenv("WEBCFACE_TRACE") != nullptr) {
        logger_internal->set_level(spdlog::level::trace);
    } else if (std::getenv("WEBCFACE_VERBOSE") != nullptr) {
        logger_internal->set_level(spdlog::level::debug);
    } else {
        logger_internal->set_level(spdlog::level::off);
    }
    logger_buf = std::make_unique<LoggerBuf>(this);
    logger_os = std::make_unique<std::ostream>(logger_buf.get());
    logger_buf_w = std::make_unique<LoggerBufW>(this);
    logger_os_w = std::make_unique<std::wostream>(logger_buf_w.get());
    log_store.setRecv(name, std::make_shared<std::vector<LogLineData<>>>());
}

Client::~Client() {
    close();
    data->join();
}
void internal::ClientData::join() {
    if (message_thread.joinable()) {
        message_thread.join();
    }
    if (connection_thread.joinable()) {
        connection_thread.join();
    }
    if (recv_thread.joinable()) {
        recv_thread.join();
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
Client &Client::onMemberEntry(std::function<void(Member)> callback) {
    std::lock_guard lock(data->event_m);
    data->member_entry_event =
        std::make_shared<std::function<void(Member)>>(std::move(callback));
    return *this;
}
std::streambuf *Client::loggerStreamBuf() { return data->logger_buf.get(); }
std::ostream &Client::loggerOStream() { return *data->logger_os.get(); }
std::wstreambuf *Client::loggerWStreamBuf() { return data->logger_buf_w.get(); }
std::wostream &Client::loggerWOStream() { return *data->logger_os_w.get(); }
std::string Client::serverVersion() const { return data->svr_version; }
std::string Client::serverName() const { return data->svr_name; }

void internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        message_push(message::packSingle(message::PingStatusReq{}));
    }
    ping_status_req = true;
}

void internal::ClientData::start() {
    std::lock_guard lock(this->connect_state_m);
    this->do_ws_init = true;
    this->connect_state_cond.notify_all();
    if (!message_thread.joinable()) {
        message_thread =
            std::thread(internal::messageThreadMain, shared_from_this());
    }
    if (!connection_thread.joinable()) {
        connection_thread =
            std::thread(internal::connectionThreadMain, shared_from_this());
    }
    if (!recv_thread.joinable() && this->auto_recv_us.load() > 0) {
        recv_thread = std::thread(internal::recvThreadMain, shared_from_this());
    }
}
void Client::close() {
    std::lock_guard lock(data->connect_state_m);
    data->closing.store(true);
    data->connect_state_cond.notify_all();
}
bool Client::connected() const {
    std::lock_guard lock(data->connect_state_m);
    return data->connected;
}
void internal::connectionThreadMain(const std::shared_ptr<ClientData> &data) {
    if (data->port <= 0) {
        return;
    }
    while (true) {
        std::unique_lock lock(data->connect_state_m);
        data->connect_state_cond.wait(lock, [&] {
            return (data->closing.load() && !data->using_curl) ||
                   data->do_ws_init ||
                   (!data->connected && !data->using_curl &&
                    data->auto_reconnect.load());
        });
        if (data->closing.load()) {
            data->using_curl = true;
            {
                ScopedUnlock un(lock);
                internal::WebSocket::close(data);
            }
            data->connected = data->current_curl_connected;
            data->using_curl = false;
            data->connect_state_cond.notify_all();
            return;
        }
        if (data->connected || data->using_curl) {
            data->do_ws_init = false;
            continue;
        }
        data->using_curl = true;
        {
            ScopedUnlock un(lock);
            internal::WebSocket::init(data);
        }
        data->connected = data->current_curl_connected;
        data->do_ws_init = false;
        data->using_curl = false;
        data->connect_state_cond.notify_all();
        if (!data->closing.load()) {
            ScopedUnlock un(lock);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}
bool internal::recvMain(const std::shared_ptr<ClientData> &data,
                        std::unique_lock<std::mutex> &lock) {
    data->using_curl = true;
    bool has_recv;
    {
        ScopedUnlock un(lock);
        has_recv =
            internal::WebSocket::recv(data, [data](const std::string &msg) {
                std::unique_lock lock(data->connect_state_m);
                data->using_curl = false;
                data->connect_state_cond.notify_all();
                {
                    ScopedUnlock un(lock);
                    data->onRecv(msg);
                }
                data->connect_state_cond.wait(lock, [&] {
                    return data->closing.load() || !data->using_curl;
                });
                data->using_curl = true;
            });
    }
    data->connected = data->current_curl_connected;
    if (!data->connected) {
        data->self_member_id = std::nullopt;
        data->sync_init_end = false;
    } else if (data->self_member_id) {
        data->sync_init_end = true;
    }
    data->using_curl = false;
    data->connect_state_cond.notify_all();
    return has_recv;
}
void Client::recvImpl(std::optional<std::chrono::microseconds> timeout) {
    auto start_t = std::chrono::steady_clock::now();
    auto next_recv_t = start_t;
    constexpr std::chrono::microseconds interval(100);
    while (true) {
        std::unique_lock lock(data->connect_state_m);
        if (!timeout || next_recv_t < start_t + *timeout) {
            // 少なくともintervalぶんは待機する
            data->connect_state_cond.wait_until(
                lock, next_recv_t, [&] { return data->closing.load(); });
        }
        // 遅くともtimeout経過したらreturnする
        if (timeout) {
            data->connect_state_cond.wait_until(lock, start_t + *timeout, [&] {
                return data->closing.load() ||
                       (data->connected && !data->using_curl);
            });
        } else {
            data->connect_state_cond.wait(lock, [&] {
                return data->closing.load() ||
                       (data->connected && !data->using_curl);
            });
        }
        if (data->closing.load() || !data->connected || data->using_curl) {
            return;
        }
        next_recv_t = std::chrono::steady_clock::now() + interval;
        bool has_recv = internal::recvMain(data, lock);
        if (has_recv) {
            return;
        }
    }
}
void internal::recvThreadMain(const std::shared_ptr<ClientData> &data) {
    if (data->port <= 0) {
        return;
    }
    auto next_recv = std::chrono::steady_clock::now();
    int recv_us = data->auto_recv_us.load();
    if (recv_us <= 0) {
        return;
    }
    while (true) {
        std::unique_lock lock(data->connect_state_m);
        data->connect_state_cond.wait_until(
            lock, next_recv, [&] { return data->closing.load(); });
        next_recv += std::chrono::microseconds(recv_us);
        data->connect_state_cond.wait(lock, [&] {
            return data->closing.load() ||
                   (data->connected && !data->using_curl);
        });
        if (data->closing.load()) {
            return;
        }
        recvMain(data, lock);
        // std::this_thread::yield();
    }
}
void internal::messageThreadMain(
    const std::shared_ptr<internal::ClientData> &data) {
    while (true) {
        std::unique_lock lock(data->connect_state_m);
        data->connect_state_cond.wait(lock, [&] {
            return data->closing.load() ||
                   (!data->using_curl && data->connected &&
                    !data->message_queue.empty());
        });
        if (data->closing.load()) {
            return;
        }
        auto msg = data->message_queue.front();
        data->message_queue.pop();
        data->using_curl = true;
        {
            ScopedUnlock un(lock);
            internal::WebSocket::send(data, msg);
        }
        data->using_curl = false;
        data->connect_state_cond.notify_all();
    }
}

void Client::start() { data->start(); }
void Client::waitConnection(std::chrono::microseconds interval) {
    data->start();
    auto next_recv = std::chrono::steady_clock::now();
    bool first_loop = true;
    while (!data->closing.load()) {
        std::unique_lock lock(data->connect_state_m);
        if (!data->connected) {
            // 初回またはautoReconnectが有効なら接続完了まで待機
            if (first_loop || data->auto_reconnect.load()) {
                data->do_ws_init = true;
                data->connect_state_cond.notify_all();
                data->connect_state_cond.wait(lock, [this] {
                    return data->closing.load() || !data->do_ws_init;
                });
            } else {
                return;
            }
        } else {
            if (data->sync_init_end) {
                return;
            } else {
                // autoRecvならsyncInit完了まで待機
                // そうでなければrecvを呼ぶ
                if (data->auto_recv_us.load() > 0) {
                    data->connect_state_cond.wait(lock, [this] {
                        return data->closing.load() || !data->connected ||
                               data->sync_init_end;
                    });
                } else {
                    data->connect_state_cond.wait_until(
                        lock, next_recv, [&] { return data->closing.load(); });
                    next_recv += interval;
                    data->connect_state_cond.wait(lock, [&] {
                        return data->closing.load() || !data->using_curl;
                    });
                    if (data->connected && !data->using_curl) {
                        recvMain(data, lock);
                    }
                }
            }
        }
        first_loop = false;
    }
}
void Client::autoRecv(bool enabled, std::chrono::microseconds interval) {
    if (enabled && interval.count() > 0) {
        data->auto_recv_us.store(static_cast<int>(interval.count()));
    } else {
        data->auto_recv_us.store(0);
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
    std::lock_guard log_lock(log_store.mtx);

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
    for (const auto &v : log_store.transferReq()) {
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
    std::lock_guard log_lock(log_store.mtx);

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

    auto log_self_opt = log_store.getRecv(self_member_name);
    if (!log_self_opt) [[unlikely]] {
        throw std::runtime_error("self log data is null");
    } else {
        auto &log_s = *log_self_opt;
        if ((log_s->size() > 0 && is_first) || log_s->size() > log_sent_lines) {
            auto begin = log_s->begin();
            auto end = log_s->end();
            if (!is_first) {
                begin += static_cast<int>(log_sent_lines);
            }
            log_sent_lines = log_s->size();
            message::pack(buffer, len, message::Log{begin, end});
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
    data->message_push(data->syncData(false));
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
    if (cl && *cl) {
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
    if (cl && *cl) {
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
        case MessageKind::sync_init_end: {
            auto r = std::any_cast<webcface::message::SyncInitEnd>(obj);
            this->svr_name = r.svr_name;
            this->svr_version = r.ver;
            this->self_member_id.emplace(r.member_id);
            break;
        }
        case MessageKind::ping: {
            this->message_push(
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
            std::shared_ptr<std::function<void(Member)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = this->ping_event[self_member_name];
            }
            if (cl && *cl) {
                cl->operator()(Field{shared_from_this(), self_member_name});
            }
            for (const auto &member_name : members) {
                {
                    std::lock_guard lock(event_m);
                    cl = findFromMap1(this->ping_event, member_name)
                             .value_or(nullptr);
                }
                if (cl && *cl) {
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
            std::shared_ptr<std::function<void(View)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap2(this->view_change_event, member, field)
                         .value_or(nullptr);
            }
            if (cl && *cl) {
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
            std::shared_ptr<std::function<void(Canvas3D)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap2(this->canvas3d_change_event, member, field)
                         .value_or(nullptr);
            }
            if (cl && *cl) {
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
            std::shared_ptr<std::function<void(Canvas2D)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap2(this->canvas2d_change_event, member, field)
                         .value_or(nullptr);
            }
            if (cl && *cl) {
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
            std::lock_guard lock(this->log_store.mtx);
            auto log_s = this->log_store.getRecv(member);
            if (!log_s) {
                log_s = std::make_shared<std::vector<LogLineData<>>>();
                this->log_store.setRecv(member, *log_s);
            }
            for (auto &lm : *r.log) {
                (*log_s)->emplace_back(lm);
            }
            std::shared_ptr<std::function<void(Log)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap1(this->log_append_event, member)
                         .value_or(nullptr);
            }
            if (cl && *cl) {
                cl->operator()(Field{shared_from_this(), member});
            }
            break;
        }
        case MessageKind::call: {
            auto r = std::any_cast<webcface::message::Call>(obj);
            auto func_info =
                this->func_store.getRecv(this->self_member_name, r.field);
            if (func_info) {
                this->message_push(webcface::message::packSingle(
                    webcface::message::CallResponse{
                        {}, r.caller_id, r.caller_member_id, true}));
                (*func_info)->run(std::move(r), shared_from_this());
            } else {
                this->message_push(webcface::message::packSingle(
                    webcface::message::CallResponse{
                        {}, r.caller_id, r.caller_member_id, false}));
            }
            break;
        }
        case MessageKind::call_response: {
            auto r = std::any_cast<webcface::message::CallResponse>(obj);
            try {
                this->func_result_store.getResult(r.caller_id)
                    ->setStarted(r.started);
                if (!r.started) {
                    this->func_result_store.removeResult(r.caller_id);
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
                        this->func_result_store.getResult(r.caller_id)
                            ->setResultException(std::current_exception());
                    }
                } else {
                    this->func_result_store.getResult(r.caller_id)
                        ->setResult(r.result);
                    // todo: 戻り値の型?
                }
                this->func_result_store.removeResult(r.caller_id);
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
            std::shared_ptr<std::function<void(Member)>> cl;
            {
                std::lock_guard lock(this->entry_m);
                cl = this->member_entry_event;
            }
            if (cl && *cl) {
                cl->operator()(Field{shared_from_this(), r.member_name});
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
            std::shared_ptr<std::function<void(Func)>> cl;
            {
                std::lock_guard lock(event_m);
                cl = findFromMap1(this->func_entry_event, member)
                         .value_or(nullptr);
            }
            if (cl && *cl) {
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
                logger_internal->warn("Invalid message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        case MessageKind::unknown:
            break;
        default:
            if (!message_kind_warned[kind]) {
                logger_internal->warn("Unknown message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        }
    }
    for (const auto &m : sync_members) {
        std::shared_ptr<std::function<void(Member)>> cl;
        {
            std::lock_guard lock(event_m);
            cl = findFromMap1(this->sync_event, m).value_or(nullptr);
        }
        if (cl && *cl) {
            cl->operator()(Field{shared_from_this(), m});
        }
    }
}
WEBCFACE_NS_END

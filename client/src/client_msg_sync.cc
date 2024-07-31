#include "webcface/client.h"
#include "webcface/message/message.h"
#include "webcface/internal/client_internal.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        std::lock_guard lock(ws_m);
        sync_queue.push(message::packSingle(message::PingStatusReq{}));
        ws_cond.notify_all();
    }
    ping_status_req = true;
}

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
    if (!log_self_opt) {
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
        if (!v.first.startsWith(field_separator)) {
            message::pack(buffer, len, v.second->toMessage(v.first));
        }
    }

    return message::packDone(buffer, len);
}
void Client::sync() {
    start();
    data->message_push(data->syncData(false));
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
const std::string &Client::serverVersion() const { return data->svr_version; }
const std::string &Client::serverName() const { return data->svr_name; }
const std::string &Client::serverHostName() const { return data->svr_hostname; }

WEBCFACE_NS_END

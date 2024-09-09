#include "webcface/client.h"
#include "webcface/message/message.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/robot_link_internal.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        std::lock_guard lock(ws_m);
        sync_queue.push(message::packSingle(message::PingStatusReq{}));
        ws_cond.notify_all();
    }
    ping_status_req = true;
}

internal::ClientData::SyncDataFirst internal::ClientData::syncDataFirst() {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard image_lock(image_store.mtx);
    std::lock_guard robot_model_lock(robot_model_store.mtx);
    std::lock_guard canvas3d_lock(canvas3d_store.mtx);
    std::lock_guard canvas2d_lock(canvas2d_store.mtx);
    std::lock_guard log_lock(log_store.mtx);

    SyncDataFirst data;
    data.value_req = value_store.transferReq();
    data.text_req = text_store.transferReq();
    data.view_req = view_store.transferReq();
    data.robot_model_req = robot_model_store.transferReq();
    data.canvas3d_req = canvas3d_store.transferReq();
    data.canvas2d_req = canvas2d_store.transferReq();
    data.image_req = image_store.transferReq();
    for (const auto &v : data.image_req) {
        for (const auto &v2 : v.second) {
            data.image_req_info[v.first][v2.first] =
                image_store.getReqInfo(v.first, v2.first);
        }
    }
    data.log_req = log_store.transferReq();
    data.ping_status_req = ping_status_req;
    data.sync_data = syncData(true);

    return data;
}
std::string internal::ClientData::packSyncDataFirst(const SyncDataFirst &data) {
    std::stringstream buffer;
    int len = 0;

    message::pack(buffer, len,
                  message::SyncInit{
                      {}, self_member_name, 0, "cpp", WEBCFACE_VERSION, ""});

    for (const auto &v : data.value_req) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::Value>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.text_req) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::Text>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.view_req) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::View>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.robot_model_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::RobotModel>{
                              {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.canvas3d_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Canvas3D>{
                              {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.canvas2d_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Canvas2D>{
                              {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.image_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Image>{
                              v.first, v2.first, v2.second,
                              data.image_req_info.at(v.first).at(v2.first)});
        }
    }
    for (const auto &v : data.log_req) {
        message::pack(buffer, len, message::LogReq{{}, v.first});
    }

    if (data.ping_status_req) {
        message::pack(buffer, len, message::PingStatusReq{});
    }

    return packSyncData(buffer, len, data.sync_data);
}
internal::ClientData::SyncDataSnapshot
internal::ClientData::syncData(bool is_first) {
    std::lock_guard value_lock(value_store.mtx);
    std::lock_guard text_lock(text_store.mtx);
    std::lock_guard view_lock(view_store.mtx);
    std::lock_guard func_lock(func_store.mtx);
    std::lock_guard image_lock(image_store.mtx);
    std::lock_guard robot_model_lock(robot_model_store.mtx);
    std::lock_guard canvas3d_lock(canvas3d_store.mtx);
    std::lock_guard canvas2d_lock(canvas2d_store.mtx);
    std::lock_guard log_lock(log_store.mtx);

    SyncDataSnapshot data;
    data.time = std::chrono::system_clock::now();

    data.value_data = value_store.transferSend(is_first);
    data.text_data = text_store.transferSend(is_first);
    data.robot_model_data = robot_model_store.transferSend(is_first);
    data.view_prev = view_store.getSendPrev(is_first);
    data.view_data = view_store.transferSend(is_first);
    data.canvas3d_prev = canvas3d_store.getSendPrev(is_first);
    data.canvas3d_data = canvas3d_store.transferSend(is_first);
    data.canvas2d_prev = canvas2d_store.getSendPrev(is_first);
    data.canvas2d_data = canvas2d_store.transferSend(is_first);
    data.image_data = image_store.transferSend(is_first);

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
            data.log_data = std::vector<LogLineData>(begin, end);
        }
    }
    data.func_data = func_store.transferSend(is_first);

    return data;
}

std::string internal::ClientData::packSyncData(std::stringstream &buffer,
                                               int &len,
                                               const SyncDataSnapshot &data) {
    message::pack(buffer, len, message::Sync{data.time});

    for (const auto &v : data.value_data) {
        message::pack(buffer, len, message::Value{{}, v.first, v.second});
    }
    for (const auto &v : data.text_data) {
        message::pack(buffer, len, message::Text{{}, v.first, v.second});
    }
    for (const auto &v : data.robot_model_data) {
        std::vector<std::shared_ptr<message::RobotLink>> links;
        links.reserve(v.second->size());
        for (std::size_t i = 0; i < v.second->size(); i++) {
            links.emplace_back(v.second->at(i));
        }
        message::pack(buffer, len, message::RobotModel{v.first, links});
    }
    for (const auto &p : data.view_data) {
        auto v_prev = data.view_prev.find(p.first);
        std::unordered_map<int, std::shared_ptr<message::ViewComponent>> v_diff;
        for (std::size_t i = 0; i < p.second->size(); i++) {
            if (v_prev == data.view_prev.end() || v_prev->second->size() <= i ||
                *(*v_prev->second)[i] != *(*p.second)[i]) {
                v_diff.emplace(static_cast<int>(i), (*p.second)[i]);
            }
        }
        if (!v_diff.empty()) {
            message::pack(buffer, len,
                          message::View{p.first, v_diff, p.second->size()});
        }
    }
    for (const auto &p : data.canvas3d_data) {
        auto v_prev = data.canvas3d_prev.find(p.first);
        std::unordered_map<int, std::shared_ptr<message::Canvas3DComponent>>
            v_diff;
        for (std::size_t i = 0; i < p.second->size(); i++) {
            if (v_prev == data.canvas3d_prev.end() ||
                v_prev->second->size() <= i ||
                *(*v_prev->second)[i] != *(*p.second)[i]) {
                v_diff.emplace(static_cast<int>(i), (*p.second)[i]);
            }
        }

        if (!v_diff.empty()) {
            message::pack(buffer, len,
                          message::Canvas3D{p.first, v_diff, p.second->size()});
        }
    }
    for (const auto &p : data.canvas2d_data) {
        auto v_prev = data.canvas2d_prev.find(p.first);
        std::unordered_map<int, std::shared_ptr<message::Canvas2DComponent>>
            v_diff;
        for (std::size_t i = 0; i < p.second->components.size(); i++) {
            if (v_prev == data.canvas2d_prev.end() ||
                v_prev->second->components.size() <= i ||
                *v_prev->second->components[i] != *p.second->components[i]) {
                v_diff.emplace(static_cast<int>(i), p.second->components[i]);
            }
        }
        if (!v_diff.empty()) {
            message::pack(buffer, len,
                          message::Canvas2D{p.first, p.second->width,
                                            p.second->height, v_diff,
                                            p.second->components.size()});
        }
    }
    for (const auto &v : data.image_data) {
        message::pack(buffer, len,
                      message::Image{v.first, v.second.toMessage()});
    }

    if (!data.log_data.empty()) {
        message::pack(buffer, len,
                      message::Log{data.log_data.begin(), data.log_data.end()});
    }
    for (const auto &v : data.func_data) {
        if (!v.first.startsWith(field_separator)) {
            message::pack(buffer, len, v.second->toMessage(v.first));
        }
    }

    return message::packDone(buffer, len);
}

std::vector<Member> Client::members() {
    return static_cast<const Client *>(this)->members();
}
std::vector<Member> Client::members() const {
    std::lock_guard lock(data->entry_m);
    std::vector<Member> ret;
    ret.reserve(data->member_entry.size());
    for (const auto &m : data->member_entry) {
        ret.push_back(member(m));
    }
    return ret;
}
const Client &
Client::onMemberEntry(std::function<void(Member)> callback) const {
    std::lock_guard lock(data->event_m);
    data->member_entry_event =
        std::make_shared<std::function<void(Member)>>(std::move(callback));
    return *this;
}
std::streambuf *Client::loggerStreamBuf() const {
    return data->logger_buf.get();
}
std::ostream &Client::loggerOStream() const { return *data->logger_os.get(); }
std::wstreambuf *Client::loggerWStreamBuf() const {
    return data->logger_buf_w.get();
}
std::wostream &Client::loggerWOStream() const {
    return *data->logger_os_w.get();
}
const std::string &Client::serverVersion() const { return data->svr_version; }
const std::string &Client::serverName() const { return data->svr_name; }
const std::string &Client::serverHostName() const { return data->svr_hostname; }

WEBCFACE_NS_END

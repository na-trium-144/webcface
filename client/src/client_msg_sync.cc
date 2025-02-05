#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/func.h"
#include "webcface/common/internal/message/log.h"
#include "webcface/client.h"
#include "webcface/common/internal/message/sync.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/common/internal/message/robot_model.h"
#include "webcface/internal/logger.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/component_internal.h"
#include "webcface/internal/robot_link_internal.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        this->messagePushReq(message::PingStatusReq{});
    }
    ping_status_req = true;
}

internal::ClientData::SyncDataFirst
internal::ClientData::SyncData::syncDataFirst(internal::ClientData *this_) {

    SyncDataFirst data;
    data.value_req = this_->value_store.transferReq();
    data.text_req = this_->text_store.transferReq();
    data.view_req = this_->view_store.transferReq();
    data.robot_model_req = this_->robot_model_store.transferReq();
    data.canvas3d_req = this_->canvas3d_store.transferReq();
    data.canvas2d_req = this_->canvas2d_store.transferReq();
    {
        std::lock_guard image_lock(this_->image_store.mtx);
        data.image_req = this_->image_store.transferReq();
        for (const auto &v : data.image_req) {
            for (const auto &v2 : v.second) {
                data.image_req_info[v.first][v2.first] =
                    this_->image_store.getReqInfo(v.first, v2.first);
            }
        }
    }
    data.log_req = this_->log_store.transferReq();
    data.ping_status_req = this_->ping_status_req;
    data.sync_data = syncData(this_, true);

    return data;
}
/// \private
template <typename T>
static void packWithLogging(const std::shared_ptr<spdlog::logger> &logger,
                            std::stringstream &buffer, int &len,
                            const T &data) {
    message::pack(buffer, len, data);
    logger->debug("-> sync data: {}", data);
}
std::string internal::ClientData::packSyncDataFirst(const SyncDataFirst &data) {
    std::stringstream buffer;
    int len = 0;

    packWithLogging(logger_internal, buffer, len,
                    message::SyncInit{
                        {}, self_member_name, 0, "cpp", WEBCFACE_VERSION, ""});

    for (const auto &v : data.value_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(
                logger_internal, buffer, len,
                message::Req<message::Value>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.text_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(
                logger_internal, buffer, len,
                message::Req<message::Text>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.view_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(
                logger_internal, buffer, len,
                message::Req<message::View>{{}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.robot_model_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(logger_internal, buffer, len,
                            message::Req<message::RobotModel>{
                                {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.canvas3d_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(logger_internal, buffer, len,
                            message::Req<message::Canvas3D>{
                                {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.canvas2d_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(logger_internal, buffer, len,
                            message::Req<message::Canvas2D>{
                                {}, v.first, v2.first, v2.second});
        }
    }
    for (const auto &v : data.image_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(logger_internal, buffer, len,
                            message::Req<message::Image>{
                                v.first, v2.first, v2.second,
                                data.image_req_info.at(v.first).at(v2.first)});
        }
    }
    for (const auto &v : data.log_req) {
        for (const auto &v2 : v.second) {
            packWithLogging(
                logger_internal, buffer, len,
                message::Req<message::Log>{{}, v.first, v2.first, v2.second});
        }
    }

    if (data.ping_status_req) {
        packWithLogging(logger_internal, buffer, len, message::PingStatusReq{});
    }

    return packSyncData(buffer, len, data.sync_data);
}
internal::ClientData::SyncDataSnapshot
internal::ClientData::SyncData::syncData(internal::ClientData *this_,
                                         bool is_first) {

    SyncDataSnapshot data;
    data.time = std::chrono::system_clock::now();

    // std::lock_guard value_lock(this_->value_store.mtx);
    data.value_data = this_->value_store.transferSend(is_first);
    // std::lock_guard text_lock(this_->text_store.mtx);
    data.text_data = this_->text_store.transferSend(is_first);
    // std::lock_guard robot_model_lock(this_->robot_model_store.mtx);
    data.robot_model_data = this_->robot_model_store.transferSend(is_first);
    {
        std::lock_guard view_lock(this_->view_store.mtx);
        data.view_prev = this_->view_store.getSendPrev(is_first);
        data.view_data = this_->view_store.transferSend(is_first);
    }
    {
        std::lock_guard canvas3d_lock(this_->canvas3d_store.mtx);
        data.canvas3d_prev = this_->canvas3d_store.getSendPrev(is_first);
        data.canvas3d_data = this_->canvas3d_store.transferSend(is_first);
    }
    {
        std::lock_guard canvas2d_lock(this_->canvas2d_store.mtx);
        data.canvas2d_prev = this_->canvas2d_store.getSendPrev(is_first);
        data.canvas2d_data = this_->canvas2d_store.transferSend(is_first);
    }
    // std::lock_guard image_lock(this_->image_store.mtx);
    data.image_data = this_->image_store.transferSend(is_first);

    {
        std::lock_guard log_lock(this_->log_store.mtx);
        for (const auto &ld : this_->log_store.transferSend(is_first)) {
            if (is_first) {
                data.log_data[ld.first] = ld.second->getAll();
            } else {
                data.log_data[ld.first] = ld.second->getDiff();
            }
        }
    }
    // std::lock_guard func_lock(this_->func_store.mtx);
    data.func_data = this_->func_store.transferSend(is_first);

    return data;
}

std::string internal::ClientData::packSyncData(std::stringstream &buffer,
                                               int &len,
                                               const SyncDataSnapshot &data) {
    packWithLogging(logger_internal, buffer, len, message::Sync{data.time});

    for (const auto &v : data.value_data) {
        packWithLogging(logger_internal, buffer, len,
                        message::Value{{}, v.first, v.second});
    }
    for (const auto &v : data.text_data) {
        packWithLogging(logger_internal, buffer, len,
                        message::Text{{}, v.first, v.second});
    }
    for (const auto &v : data.robot_model_data) {
        std::vector<std::shared_ptr<message::RobotLink>> links;
        links.reserve(v.second->size());
        for (std::size_t i = 0; i < v.second->size(); i++) {
            links.emplace_back(v.second->at(i));
        }
        packWithLogging(logger_internal, buffer, len,
                        message::RobotModel{v.first, links});
    }
    for (const auto &p : data.view_data) {
        auto v_prev = data.view_prev.find(p.first);
        std::map<std::string, std::shared_ptr<message::ViewComponentData>>
            v_diff;
        for (const auto &id : p.second->data_ids) {
            if (v_prev == data.view_prev.end() ||
                v_prev->second->components.count(id.u8String()) == 0 ||
                *v_prev->second->components.at(id.u8String()) !=
                    *p.second->components.at(id.u8String())) {
                v_diff.emplace(id.u8String(),
                               p.second->components.at(id.u8String()));
            }
        }
        std::optional<std::vector<SharedString>> v_ids_changed = std::nullopt;
        if (v_prev == data.view_prev.end() ||
            v_prev->second->data_ids != p.second->data_ids) {
            v_ids_changed.emplace(p.second->data_ids);
        }
        if (!v_diff.empty() || v_ids_changed) {
            packWithLogging(logger_internal, buffer, len,
                            message::View{p.first, std::move(v_diff),
                                          std::move(v_ids_changed)});
        }
    }
    for (const auto &p : data.canvas3d_data) {
        auto v_prev = data.canvas3d_prev.find(p.first);
        std::map<std::string, std::shared_ptr<message::Canvas3DComponentData>>
            v_diff;
        for (const auto &id : p.second->data_ids) {
            if (v_prev == data.canvas3d_prev.end() ||
                v_prev->second->components.count(id.u8String()) == 0 ||
                *v_prev->second->components.at(id.u8String()) !=
                    *p.second->components.at(id.u8String())) {
                v_diff.emplace(id.u8String(),
                               p.second->components.at(id.u8String()));
            }
        }
        std::optional<std::vector<SharedString>> v_ids_changed = std::nullopt;
        if (v_prev == data.canvas3d_prev.end() ||
            v_prev->second->data_ids != p.second->data_ids) {
            v_ids_changed.emplace(p.second->data_ids);
        }
        if (!v_diff.empty() || v_ids_changed) {
            packWithLogging(logger_internal, buffer, len,
                            message::Canvas3D{p.first, std::move(v_diff),
                                              std::move(v_ids_changed)});
        }
    }
    for (const auto &p : data.canvas2d_data) {
        auto v_prev = data.canvas2d_prev.find(p.first);
        std::map<std::string, std::shared_ptr<message::Canvas2DComponentData>>
            v_diff;
        for (const auto &id : p.second->data_ids) {
            if (v_prev == data.canvas2d_prev.end() ||
                v_prev->second->components.count(id.u8String()) == 0 ||
                *v_prev->second->components.at(id.u8String()) !=
                    *p.second->components.at(id.u8String())) {
                v_diff.emplace(id.u8String(),
                               p.second->components.at(id.u8String()));
            }
        }
        std::optional<std::vector<SharedString>> v_ids_changed;
        if (v_prev == data.canvas2d_prev.end() ||
            v_prev->second->data_ids != p.second->data_ids) {
            v_ids_changed.emplace(p.second->data_ids);
        }
        if (!v_diff.empty() || v_ids_changed) {
            packWithLogging(
                logger_internal, buffer, len,
                message::Canvas2D{p.first, p.second->width, p.second->height,
                                  std::move(v_diff), std::move(v_ids_changed)});
        }
    }
    for (const auto &v : data.image_data) {
        packWithLogging(logger_internal, buffer, len,
                        message::Image{v.first, v.second.toMessage()});
    }

    for (const auto &v : data.log_data) {
        packWithLogging(
            logger_internal, buffer, len,
            message::Log{v.first, v.second.begin(), v.second.end()});
    }
    for (const auto &v : data.func_data) {
        if (!v.first.startsWith(field_separator)) {
            packWithLogging(logger_internal, buffer, len,
                            v.second->toMessage(v.first));
        }
    }

    return message::packDone(buffer, len);
}

std::vector<Member> Client::members() {
    return static_cast<const Client *>(this)->members();
}
std::vector<Member> Client::members() const {
    auto lock_entry = data->member_entry.shared_lock();
    std::vector<Member> ret;
    ret.reserve(lock_entry->size());
    for (const auto &m : lock_entry.get()) {
        ret.push_back(member(m.first));
    }
    return ret;
}
const Client &
Client::onMemberEntry(std::function<void(Member)> callback) const {
    data->member_entry_event =
        std::make_shared<std::function<void(Member)>>(std::move(callback));
    return *this;
}
// \private
static std::streambuf *
getLoggerBuf(const std::shared_ptr<internal::ClientData> &data,
             const SharedString &field) {
    auto lock_logger_buf = data->logger_buf.lock();
    if (!lock_logger_buf->count(field)) {
        lock_logger_buf->emplace(
            field, std::make_unique<LoggerBuf>(data.get(), field));
    }
    return lock_logger_buf->at(field).get();
}
std::streambuf *Client::loggerStreamBuf() const {
    return getLoggerBuf(data, message::Log::defaultLogName());
}
std::streambuf *Client::loggerStreamBuf(std::string_view name) const {
    return getLoggerBuf(data, SharedString::encode(name));
}
// \private
static std::wstreambuf *
getLoggerBufW(const std::shared_ptr<internal::ClientData> &data,
              const SharedString &field) {
    auto lock_logger_buf = data->logger_buf_w.lock();
    if (!lock_logger_buf->count(field)) {
        lock_logger_buf->emplace(
            field, std::make_unique<LoggerBufW>(data.get(), field));
    }
    return lock_logger_buf->at(field).get();
}
std::wstreambuf *Client::loggerWStreamBuf() const {
    return getLoggerBufW(data, message::Log::defaultLogName());
}
std::wstreambuf *Client::loggerWStreamBuf(std::wstring_view name) const {
    return getLoggerBufW(data, SharedString::encode(name));
}
// \private
static std::ostream &
getLoggerOS(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &field) {
    auto lock_logger_os = data->logger_os.lock();
    if (!lock_logger_os->count(field)) {
        lock_logger_os->emplace(
            field, std::make_unique<std::ostream>(getLoggerBuf(data, field)));
    }
    return *lock_logger_os->at(field);
}
std::ostream &Client::loggerOStream() const {
    return getLoggerOS(data, message::Log::defaultLogName());
}
std::ostream &Client::loggerOStream(std::string_view name) const {
    return getLoggerOS(data, SharedString::encode(name));
}
// \private
static std::wostream &
getLoggerWOS(const std::shared_ptr<internal::ClientData> &data,
             const SharedString &field) {
    auto lock_logger_os = data->logger_os_w.lock();
    if (!lock_logger_os->count(field)) {
        lock_logger_os->emplace(
            field, std::make_unique<std::wostream>(getLoggerBufW(data, field)));
    }
    return *lock_logger_os->at(field);
}
std::wostream &Client::loggerWOStream() const {
    return getLoggerWOS(data, message::Log::defaultLogName());
}
std::wostream &Client::loggerWOStream(std::wstring_view name) const {
    return getLoggerWOS(data, SharedString::encode(name));
}
const std::string &Client::serverVersion() const {
    return data->svr_version.shared_lock().get();
}
const std::string &Client::serverName() const {
    return data->svr_name.shared_lock().get();
}
const std::string &Client::serverHostName() const {
    return data->svr_hostname.shared_lock().get();
}

WEBCFACE_NS_END

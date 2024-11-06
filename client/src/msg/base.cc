#include "webcface/internal/client_internal.h"
#include "webcface/member.h"
#include "webcface/message/base.h"
#include "webcface/message/sync.h"

WEBCFACE_NS_BEGIN


void internal::ClientData::onRecv(
    const std::vector<std::pair<int, std::shared_ptr<void>>> &messages) {
    static std::unordered_map<int, bool> message_kind_warned;
    namespace MessageKind = webcface::message::MessageKind;
    std::vector<SharedString> sync_members;
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::sync_init_end:
        case MessageKind::ping:
        case MessageKind::ping_status:
        case MessageKind::sync:
        case MessageKind::sync_init:
            onRecv_sync(kind, obj, sync_members);
            break;
        case MessageKind::value + MessageKind::res:
        case MessageKind::value + MessageKind::entry:
            onRecv_value(kind, obj);
            break;
        case MessageKind::text + MessageKind::res:
        case MessageKind::text + MessageKind::entry:
            onRecv_text(kind, obj);
            break;
        case MessageKind::view + MessageKind::res:
        case MessageKind::view + MessageKind::entry:
            onRecv_view(kind, obj);
            break;
        case MessageKind::image + MessageKind::res:
        case MessageKind::image + MessageKind::entry:
            onRecv_image(kind, obj);
            break;
        case MessageKind::canvas3d + MessageKind::res:
        case MessageKind::canvas3d + MessageKind::entry:
            onRecv_canvas3d(kind, obj);
            break;
        case MessageKind::canvas2d + MessageKind::res:
        case MessageKind::canvas2d + MessageKind::entry:
            onRecv_canvas2d(kind, obj);
            break;
        case MessageKind::robot_model + MessageKind::res:
        case MessageKind::robot_model + MessageKind::entry:
            onRecv_robot_model(kind, obj);
            break;
        case MessageKind::log + MessageKind::res:
        case MessageKind::log + MessageKind::entry:
            onRecv_log(kind, obj);
            break;
        case MessageKind::call:
        case MessageKind::call_response:
        case MessageKind::call_result:
        case MessageKind::func_info:
            onRecv_func(kind, obj);
            break;
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
        case MessageKind::log + MessageKind::req:
        case MessageKind::log_default:
        case MessageKind::log_req_default:
            if (!message_kind_warned[kind]) {
                logger_internal->warn("Invalid message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        case MessageKind::log_entry_default:
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

void internal::ClientData::pingStatusReq() {
    if (!ping_status_req) {
        this->messagePushReq(message::packSingle(message::PingStatusReq{}));
    }
    ping_status_req = true;
}

internal::ClientData::SyncDataFirst
internal::ClientData::SyncMutexedData::syncDataFirst(
    internal::ClientData *this_) {

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
std::string internal::ClientData::packSyncDataFirst(const SyncDataFirst &data) {
    std::stringstream buffer;
    int len = 0;

    message::pack(buffer, len,
                  message::SyncInit{
                      {}, self_member_name, 0, "cpp", WEBCFACE_VERSION, ""});

    packSyncDataFirst_value(buffer, len, data);
    packSyncDataFirst_text(buffer, len, data);
    packSyncDataFirst_view(buffer, len, data);
    packSyncDataFirst_canvas3d(buffer, len, data);
    packSyncDataFirst_canvas2d(buffer, len, data);
    packSyncDataFirst_image(buffer, len, data);
    packSyncDataFirst_log(buffer, len, data);
    packSyncDataFirst_robot_model(buffer, len, data);

    if (data.ping_status_req) {
        message::pack(buffer, len, message::PingStatusReq{});
    }

    return packSyncData(buffer, len, data.sync_data);
}
internal::ClientData::SyncDataSnapshot
internal::ClientData::SyncMutexedData::syncData(internal::ClientData *this_,
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
    message::pack(buffer, len, message::Sync{data.time});

    packSyncData_value(buffer, len, data);
    packSyncData_text(buffer, len, data);
    packSyncData_view(buffer, len, data);
    packSyncData_canvas3d(buffer, len, data);
    packSyncData_canvas2d(buffer, len, data);
    packSyncData_image(buffer, len, data);
    packSyncData_log(buffer, len, data);
    packSyncData_robot_model(buffer, len, data);
    packSyncData_func(buffer, len, data);

    return message::packDone(buffer, len);
}

WEBCFACE_NS_END

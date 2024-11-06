#include "webcface/internal/client_internal.h"
#include "webcface/member.h"
#include "webcface/message/sync.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_sync(int kind,
                                       const std::shared_ptr<void> &obj,
                                       std::vector<SharedString> &sync_members) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::sync_init_end: {
        auto &r = *static_cast<webcface::message::SyncInitEnd *>(obj.get());
        this->svr_name = r.svr_name;
        this->svr_version = r.ver;
        this->self_member_id.emplace(r.member_id);
        this->svr_hostname = r.hostname;
        {
            ScopedWsLock lock_ws(this);
            lock_ws.getData().sync_init_end = true;
            this->ws_cond.notify_all();
        }
        break;
    }
    case MessageKind::ping: {
        this->messagePushOnline(
            webcface::message::packSingle(webcface::message::Ping{}));
        break;
    }
    case MessageKind::ping_status: {
        auto &r = *static_cast<webcface::message::PingStatus *>(obj.get());
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
        auto &r = *static_cast<webcface::message::Sync *>(obj.get());
        const auto &member = this->getMemberNameFromId(r.member_id);
        this->sync_time_store.setRecv(member, r.getTime());
        sync_members.push_back(member);
        break;
    }
        case MessageKind::sync_init: {
            auto &r = *static_cast<webcface::message::SyncInit *>(obj.get());
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
            this->log_store.clearEntry(r.member_name);
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
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}


WEBCFACE_NS_END

#include "webcface/internal/client_internal.h"
#include "webcface/log.h"
#include "webcface/message/log.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_log(int kind,
                                      const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::log + MessageKind::res: {
        auto &r =
            *static_cast<webcface::message::Res<webcface::message::Log> *>(
                obj.get());
        std::lock_guard lock_s(this->log_store.mtx);
        auto [member, field] = this->log_store.getReq(r.req_id, r.sub_field);
        auto log_s = this->log_store.getRecv(member, field);
        if (!log_s) {
            log_s = std::make_shared<LogData>();
            this->log_store.setRecv(member, field, *log_s);
        }
        int log_keep_lines_local = log_keep_lines.load();
        auto r_begin = r.log->begin();
        auto r_end = r.log->end();
        if (log_keep_lines_local >= 0) {
            if (r.log->size() >
                static_cast<std::size_t>(log_keep_lines_local)) {
                r_begin = r_end - log_keep_lines_local;
            }
            while ((*log_s)->data.size() + (r_end - r_begin) >
                   static_cast<std::size_t>(log_keep_lines_local)) {
                (*log_s)->data.pop_front();
            }
        }
        for (auto lit = r_begin; lit != r_end; lit++) {
            (*log_s)->data.emplace_back(*lit);
        }
        std::shared_ptr<std::function<void(Log)>> cl;
        {
            std::lock_guard lock(event_m);
            cl = findFromMap2(this->log_append_event, member, field)
                     .value_or(nullptr);
        }
        if (cl && *cl) {
            cl->operator()(Field{shared_from_this(), member, field});
        }
        break;
    }
    case MessageKind::entry + MessageKind::log: {
        auto &r =
            *static_cast<webcface::message::Entry<webcface::message::Log> *>(
                obj.get());
        onRecvEntry(this, r, this->log_store, this->log_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_log(std::stringstream &buffer,
                                                 int &len,
                                                 const SyncDataFirst &data) {
    for (const auto &v : data.log_req) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::Log>{{}, v.first, v2.first, v2.second});
        }
    }
}
void internal::ClientData::packSyncData_log(std::stringstream &buffer, int &len,
                                            const SyncDataSnapshot &data) {
    for (const auto &v : data.log_data) {
        message::pack(buffer, len,
                      message::Log{v.first, v.second.begin(), v.second.end()});
    }
}

WEBCFACE_NS_END

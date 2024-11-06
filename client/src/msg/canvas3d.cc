#include "webcface/canvas3d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/component_internal.h"
#include "webcface/message/canvas3d.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_canvas3d(int kind,
                                           const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::canvas3d + MessageKind::res: {
        auto &r =
            *static_cast<webcface::message::Res<webcface::message::Canvas3D> *>(
                obj.get());
        std::lock_guard lock_s(this->canvas3d_store.mtx);
        auto [member, field] =
            this->canvas3d_store.getReq(r.req_id, r.sub_field);
        auto v_prev = this->canvas3d_store.getRecv(member, field);
        std::shared_ptr<
            std::vector<std::shared_ptr<internal::Canvas3DComponentData>>>
            vv_prev;
        if (v_prev) {
            vv_prev = *v_prev;
        } else {
            vv_prev = std::make_shared<
                std::vector<std::shared_ptr<internal::Canvas3DComponentData>>>(
                r.length);
            v_prev.emplace(vv_prev);
            this->canvas3d_store.setRecv(member, field, vv_prev);
        }
        vv_prev->resize(r.length);
        for (const auto &d : r.data_diff) {
            (*vv_prev)[std::stoi(d.first)] =
                std::make_shared<internal::Canvas3DComponentData>(*d.second);
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
    case MessageKind::entry + MessageKind::canvas3d: {
        auto &r = *static_cast<
            webcface::message::Entry<webcface::message::Canvas3D> *>(obj.get());
        onRecvEntry(this, r, this->canvas3d_store, this->canvas3d_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_canvas3d(std::stringstream &buffer,
                                                   int &len,
                                                   const SyncDataFirst &data) {
    for (const auto &v : data.canvas3d_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Canvas3D>{
                              {}, v.first, v2.first, v2.second});
        }
    }
}
void internal::ClientData::packSyncData_canvas3d(std::stringstream &buffer,
                                              int &len,
                                              const SyncDataSnapshot &data) {
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
}

WEBCFACE_NS_END

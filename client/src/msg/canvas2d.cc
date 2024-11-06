#include "webcface/canvas2d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/component_internal.h"
#include "webcface/message/canvas2d.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_canvas2d(int kind,
                                           const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::canvas2d + MessageKind::res: {
        auto &r =
            *static_cast<webcface::message::Res<webcface::message::Canvas2D> *>(
                obj.get());
        std::lock_guard lock_s(this->canvas2d_store.mtx);
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
        for (const auto &d : r.data_diff) {
            vv_prev->components[std::stoi(d.first)] =
                std::make_shared<internal::Canvas2DComponentData>(*d.second);
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
    case MessageKind::entry + MessageKind::canvas2d: {
        auto &r = *static_cast<
            webcface::message::Entry<webcface::message::Canvas2D> *>(obj.get());
        onRecvEntry(this, r, this->canvas2d_store, this->canvas2d_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_canvas2d(
    std::stringstream &buffer, int &len, const SyncDataFirst &data) {
    for (const auto &v : data.canvas2d_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Canvas2D>{
                              {}, v.first, v2.first, v2.second});
        }
    }
}
void internal::ClientData::packSyncData_canvas2d(std::stringstream &buffer,
                                                 int &len,
                                                 const SyncDataSnapshot &data) {
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
}

WEBCFACE_NS_END

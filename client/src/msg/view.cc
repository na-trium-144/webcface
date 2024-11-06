#include "webcface/internal/client_internal.h"
#include "webcface/view.h"
#include "webcface/internal/component_internal.h"
#include "webcface/message/view.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_view(int kind,
                                       const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::view + MessageKind::res: {
        auto &r =
            *static_cast<webcface::message::Res<webcface::message::View> *>(
                obj.get());
        std::lock_guard lock_s(this->view_store.mtx);
        auto [member, field] = this->view_store.getReq(r.req_id, r.sub_field);
        auto v_prev = this->view_store.getRecv(member, field);
        std::shared_ptr<
            std::vector<std::shared_ptr<internal::ViewComponentData>>>
            vv_prev;
        if (v_prev) {
            vv_prev = *v_prev;
        } else {
            vv_prev = std::make_shared<
                std::vector<std::shared_ptr<internal::ViewComponentData>>>(
                r.length);
            v_prev.emplace(vv_prev);
            this->view_store.setRecv(member, field, vv_prev);
        }
        vv_prev->resize(r.length);
        for (const auto &d : r.data_diff) {
            (*vv_prev)[std::stoi(d.first)] =
                std::make_shared<internal::ViewComponentData>(*d.second);
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
    case MessageKind::entry + MessageKind::view: {
        auto &r =
            *static_cast<webcface::message::Entry<webcface::message::View> *>(
                obj.get());
        onRecvEntry(this, r, this->view_store, this->view_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_view(std::stringstream &buffer,
                                                  int &len,
                                                  const SyncDataFirst &data) {
    for (const auto &v : data.view_req) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::View>{{}, v.first, v2.first, v2.second});
        }
    }
}
void internal::ClientData::packSyncData_view(std::stringstream &buffer,
                                             int &len,
                                             const SyncDataSnapshot &data) {
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
}

WEBCFACE_NS_END

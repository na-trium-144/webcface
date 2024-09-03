#include "webcface/image.h"
#include "webcface/log.h"
#include "webcface/member.h"
#include "webcface/value.h"
#include "webcface/view.h"
#include "webcface/func.h"
#include "webcface/canvas3d.h"
#include "webcface/canvas2d.h"
#include "webcface/message/message.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/robot_link_internal.h"

WEBCFACE_NS_BEGIN

/// \private
template <typename M, typename K1, typename K2>
static auto findFromMap2(const M &map, const K1 &key1, const K2 &key2)
    -> std::optional<std::decay_t<decltype(map.at(key1).at(key2))>> {
    auto s_it = map.find(key1);
    if (s_it != map.end()) {
        auto it = s_it->second.find(key2);
        if (it != s_it->second.end()) {
            return it->second;
        }
    }
    return std::nullopt;
}
/// \private
template <typename M, typename K1>
static auto findFromMap1(const M &map, const K1 &key1)
    -> std::optional<std::decay_t<decltype(map.at(key1))>> {
    auto it = map.find(key1);
    if (it != map.end()) {
        return it->second;
    }
    return std::nullopt;
}
/// \private
template <typename Msg, typename T, typename S, typename E>
static void onRecvRes(internal::ClientData *this_, const Msg &r, const T &data,
                      S &store, const E &event) {
    auto [member, field] = store.getReq(r.req_id, r.sub_field);
    store.setRecv(member, field, data);
    std::decay_t<decltype(event.at(member).at(field))> cl;
    {
        std::lock_guard lock(this_->event_m);
        cl = findFromMap2(event, member, field).value_or(nullptr);
    }
    if (cl && *cl) {
        cl->operator()(Field{this_->shared_from_this(), member, field});
    }
}
/// \private
template <typename Msg, typename S, typename E>
static void onRecvEntry(internal::ClientData *this_, const Msg &r, S &store,
                        const E &event) {
    auto member = this_->getMemberNameFromId(r.member_id);
    store.setEntry(member, r.field);
    std::decay_t<decltype(event.at(member))> cl;
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
            auto &r = *static_cast<webcface::message::SyncInitEnd *>(obj.get());
            this->svr_name = r.svr_name;
            this->svr_version = r.ver;
            this->self_member_id.emplace(r.member_id);
            this->svr_hostname = r.hostname;
            {
                std::lock_guard lock(this->ws_m);
                this->sync_init_end = true;
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
        case MessageKind::value + MessageKind::res: {
            auto &r = *static_cast<
                webcface::message::Res<webcface::message::Value> *>(obj.get());
            onRecvRes(this, r, r.data, this->value_store,
                      this->value_change_event);
            break;
        }
        case MessageKind::text + MessageKind::res: {
            auto &r =
                *static_cast<webcface::message::Res<webcface::message::Text> *>(
                    obj.get());
            onRecvRes(this, r, r.data, this->text_store,
                      this->text_change_event);
            break;
        }
        case MessageKind::robot_model + MessageKind::res: {
            auto &r =
                *static_cast<message::Res<message::RobotModel> *>(obj.get());
            auto links_data = std::make_shared<
                std::vector<std::shared_ptr<internal::RobotLinkData>>>();
            links_data->reserve(r.data.size());
            for (std::size_t i = 0; i < r.data.size(); i++) {
                links_data->push_back(std::make_shared<internal::RobotLinkData>(
                    *r.data[i], *links_data));
            }
            onRecvRes(this, r, links_data, this->robot_model_store,
                      this->robot_model_change_event);
            break;
        }
        case MessageKind::view + MessageKind::res: {
            auto &r =
                *static_cast<webcface::message::Res<webcface::message::View> *>(
                    obj.get());
            std::lock_guard lock_s(this->view_store.mtx);
            auto [member, field] =
                this->view_store.getReq(r.req_id, r.sub_field);
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
        case MessageKind::canvas3d + MessageKind::res: {
            auto &r = *static_cast<
                webcface::message::Res<webcface::message::Canvas3D> *>(
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
                vv_prev = std::make_shared<std::vector<
                    std::shared_ptr<internal::Canvas3DComponentData>>>(
                    r.length);
                v_prev.emplace(vv_prev);
                this->canvas3d_store.setRecv(member, field, vv_prev);
            }
            vv_prev->resize(r.length);
            for (const auto &d : r.data_diff) {
                (*vv_prev)[std::stoi(d.first)] =
                    std::make_shared<internal::Canvas3DComponentData>(
                        *d.second);
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
            auto &r = *static_cast<
                webcface::message::Res<webcface::message::Canvas2D> *>(
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
                    std::make_shared<internal::Canvas2DComponentData>(
                        *d.second);
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
            auto &r = *static_cast<
                webcface::message::Res<webcface::message::Image> *>(obj.get());
            onRecvRes(this, r, r, this->image_store, this->image_change_event);
            break;
        }
        case MessageKind::log: {
            auto &r = *static_cast<webcface::message::Log *>(obj.get());
            auto member = this->getMemberNameFromId(r.member_id);
            std::lock_guard lock_s(this->log_store.mtx);
            auto log_s = this->log_store.getRecv(member);
            if (!log_s) {
                log_s = std::make_shared<std::vector<LogLineData>>();
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
            auto &r = *static_cast<webcface::message::Call *>(obj.get());
            auto func_info =
                this->func_store.getRecv(this->self_member_name, r.field);
            if (func_info) {
                this->messagePushAlways(webcface::message::packSingle(
                    webcface::message::CallResponse{
                        {}, r.caller_id, r.caller_member_id, true}));
                (*func_info)->run(std::move(r));
            } else {
                this->messagePushAlways(webcface::message::packSingle(
                    webcface::message::CallResponse{
                        {}, r.caller_id, r.caller_member_id, false}));
            }
            break;
        }
        case MessageKind::call_response: {
            auto &r =
                *static_cast<webcface::message::CallResponse *>(obj.get());
            try {
                this->func_result_store.getResult(r.caller_id)
                    ->setter()
                    .reach(r.started);
                if (!r.started) {
                    this->func_result_store.removeResult(r.caller_id);
                }
            } catch (const std::runtime_error &e) {
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
            auto &r = *static_cast<webcface::message::CallResult *>(obj.get());
            try {
                if (r.is_error) {
                    this->func_result_store.getResult(r.caller_id)
                        ->setter()
                        .reject(r.result);
                } else {
                    this->func_result_store.getResult(r.caller_id)
                        ->setter()
                        .respond(r.result);
                    // todo: 戻り値の型?
                }
                this->func_result_store.removeResult(r.caller_id);
            } catch (const std::runtime_error &e) {
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
        case MessageKind::entry + MessageKind::value: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Value> *>(
                obj.get());
            onRecvEntry(this, r, this->value_store, this->value_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::text: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Text> *>(obj.get());
            onRecvEntry(this, r, this->text_store, this->text_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::view: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::View> *>(obj.get());
            onRecvEntry(this, r, this->view_store, this->view_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::canvas3d: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Canvas3D> *>(
                obj.get());
            onRecvEntry(this, r, this->canvas3d_store,
                        this->canvas3d_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::canvas2d: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Canvas2D> *>(
                obj.get());
            onRecvEntry(this, r, this->canvas2d_store,
                        this->canvas2d_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::robot_model: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::RobotModel> *>(
                obj.get());
            onRecvEntry(this, r, this->robot_model_store,
                        this->robot_model_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::image: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Image> *>(
                obj.get());
            onRecvEntry(this, r, this->image_store, this->image_entry_event);
            break;
        }
        case MessageKind::log_entry: {
            auto &r = *static_cast<webcface::message::LogEntry *>(obj.get());
            auto member = this->getMemberNameFromId(r.member_id);
            this->log_store.setEntry(member);
            // std::decay_t<decltype(this->log_entry_event.at(member))> cl;
            // {
            //     std::lock_guard lock(this->event_m);
            //     cl = findFromMap1(this->log_entry_event,
            //     member).value_or(nullptr);
            // }
            // if (cl && *cl) {
            //     cl->operator()(Field{this->shared_from_this(), member});
            // }
            break;
        }
        case MessageKind::func_info: {
            auto &r = *static_cast<webcface::message::FuncInfo *>(obj.get());
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

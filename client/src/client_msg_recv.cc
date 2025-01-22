#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/func.h"
#include "webcface/common/internal/message/log.h"
#include "webcface/common/internal/message/sync.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/common/internal/message/plot.h"
#include "webcface/log.h"
#include "webcface/member.h"
#include "webcface/view.h"
#include "webcface/func.h"
#include "webcface/image.h"
#include "webcface/value.h"
#include "webcface/text.h"
#include "webcface/canvas3d.h"
#include "webcface/canvas2d.h"
#include "webcface/plot.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/component_internal.h"
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
static void onRecvRes(internal::ClientData *this_, const Msg &r, T &&data,
                      S &store, const E &event) {
    auto [member, field] = store.getReq(r.req_id, r.sub_field);
    store.setRecv(member, field, std::forward<T>(data));
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

void internal::ClientData::onRecv(
    const std::vector<std::pair<int, std::shared_ptr<void>>> &messages) {
    static std::unordered_map<int, bool> message_kind_warned;
    namespace MessageKind = webcface::message::MessageKind;
    std::vector<SharedString> sync_members;
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::sync_init_end: {
            auto &r = *static_cast<webcface::message::SyncInitEnd *>(obj.get());
            this->logger_internal->debug("received {}", r);
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
            this->logger_internal->debug("received {}", message::Ping{});
            this->messagePushOnline(webcface::message::Ping{});
            break;
        }
        case MessageKind::ping_status: {
            auto &r = *static_cast<webcface::message::PingStatus *>(obj.get());
            this->logger_internal->debug("received {}", r);
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
            this->logger_internal->debug("received {}", r);
            const auto &member = this->getMemberNameFromId(r.member_id);
            this->sync_time_store.setRecv(member, r.getTime());
            sync_members.push_back(member);
            break;
        }
        case MessageKind::value + MessageKind::res: {
            auto &r = *static_cast<
                webcface::message::Res<webcface::message::Value> *>(obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvRes(this, r, r.data, this->value_store,
                      this->value_change_event);
            break;
        }
        case MessageKind::text + MessageKind::res: {
            auto &r =
                *static_cast<webcface::message::Res<webcface::message::Text> *>(
                    obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvRes(this, r, r.data, this->text_store,
                      this->text_change_event);
            break;
        }
        case MessageKind::robot_model + MessageKind::res: {
            auto &r =
                *static_cast<message::Res<message::RobotModel> *>(obj.get());
            this->logger_internal->debug("received {}", r);
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
        case MessageKind::plot + MessageKind::res: {
            auto &r = *static_cast<message::Res<message::Plot> *>(obj.get());
            onRecvRes(this, r, std::move(r.data), this->plot_store,
                      this->plot_change_event);
            break;
        }
        case MessageKind::view + MessageKind::res: {
            auto &r =
                *static_cast<webcface::message::Res<webcface::message::View> *>(
                    obj.get());
            this->logger_internal->debug("received {}", r);
            std::lock_guard lock_s(this->view_store.mtx);
            auto [member, field] =
                this->view_store.getReq(r.req_id, r.sub_field);
            auto v_prev = this->view_store.getRecv(member, field);
            std::shared_ptr<message::ViewData> vb_prev;
            if (v_prev) {
                vb_prev = *v_prev;
            } else {
                vb_prev = std::make_shared<message::ViewData>();
                v_prev.emplace(vb_prev);
                this->view_store.setRecv(member, field, vb_prev);
            }
            if (r.data_ids) {
                vb_prev->data_ids = std::move(*r.data_ids);
            }
            for (const auto &d : r.data_diff) {
                auto id = SharedString::fromU8String(d.first);
                vb_prev->components[id.u8String()] = d.second;
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
            this->logger_internal->debug("received {}", r);
            std::lock_guard lock_s(this->canvas3d_store.mtx);
            auto [member, field] =
                this->canvas3d_store.getReq(r.req_id, r.sub_field);
            auto v_prev = this->canvas3d_store.getRecv(member, field);
            std::shared_ptr<message::Canvas3DData> vv_prev;
            if (v_prev) {
                vv_prev = *v_prev;
            } else {
                vv_prev = std::make_shared<message::Canvas3DData>();
                v_prev.emplace(vv_prev);
                this->canvas3d_store.setRecv(member, field, vv_prev);
            }
            if (r.data_ids) {
                vv_prev->data_ids = std::move(*r.data_ids);
            }
            for (const auto &d : r.data_diff) {
                auto id = SharedString::fromU8String(d.first);
                vv_prev->components[id.u8String()] = d.second;
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
            this->logger_internal->debug("received {}", r);
            std::lock_guard lock_s(this->canvas2d_store.mtx);
            auto [member, field] =
                this->canvas2d_store.getReq(r.req_id, r.sub_field);
            auto v_prev = this->canvas2d_store.getRecv(member, field);
            std::shared_ptr<message::Canvas2DData> vv_prev;
            if (v_prev) {
                vv_prev = *v_prev;
            } else {
                vv_prev = std::make_shared<message::Canvas2DData>();
                v_prev.emplace(vv_prev);
                this->canvas2d_store.setRecv(member, field, vv_prev);
            }
            vv_prev->width = r.width;
            vv_prev->height = r.height;
            if (r.data_ids) {
                vv_prev->data_ids = std::move(*r.data_ids);
            }
            for (const auto &d : r.data_diff) {
                auto id = SharedString::fromU8String(d.first);
                vv_prev->components[id.u8String()] = d.second;
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
            this->logger_internal->debug("received {}", r);
            onRecvRes(this, r, r, this->image_store, this->image_change_event);
            break;
        }
        case MessageKind::log + MessageKind::res: {
            auto &r =
                *static_cast<webcface::message::Res<webcface::message::Log> *>(
                    obj.get());
            this->logger_internal->debug("received {}", r);
            std::lock_guard lock_s(this->log_store.mtx);
            auto [member, field] =
                this->log_store.getReq(r.req_id, r.sub_field);
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
        case MessageKind::call: {
            auto &r = *static_cast<webcface::message::Call *>(obj.get());
            this->logger_internal->debug("received {}", r);
            auto func_info =
                this->func_store.getRecv(this->self_member_name, r.field);
            if (func_info) {
                this->messagePushAlways(webcface::message::CallResponse{
                    {}, r.caller_id, r.caller_member_id, true});
                (*func_info)->run(std::move(r));
            } else {
                this->messagePushAlways(webcface::message::CallResponse{
                    {}, r.caller_id, r.caller_member_id, false});
            }
            break;
        }
        case MessageKind::call_response: {
            auto &r =
                *static_cast<webcface::message::CallResponse *>(obj.get());
            this->logger_internal->debug("received {}", r);
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
            this->logger_internal->debug("received {}", r);
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
            this->logger_internal->debug("received {}", r);
            {
                std::lock_guard lock(this->entry_m);
                this->member_entry.emplace(r.member_name);
            }
            this->value_store.initMember(r.member_name);
            this->text_store.initMember(r.member_name);
            this->func_store.initMember(r.member_name);
            this->view_store.initMember(r.member_name);
            this->image_store.initMember(r.member_name);
            this->robot_model_store.initMember(r.member_name);
            this->plot_store.initMember(r.member_name);
            this->canvas3d_store.initMember(r.member_name);
            this->canvas2d_store.initMember(r.member_name);
            this->log_store.initMember(r.member_name);
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
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->value_store, this->value_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::text: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Text> *>(obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->text_store, this->text_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::view: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::View> *>(obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->view_store, this->view_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::canvas3d: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Canvas3D> *>(
                obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->canvas3d_store,
                        this->canvas3d_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::canvas2d: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Canvas2D> *>(
                obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->canvas2d_store,
                        this->canvas2d_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::robot_model: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::RobotModel> *>(
                obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->robot_model_store,
                        this->robot_model_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::plot: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Plot> *>(obj.get());
            onRecvEntry(this, r, this->plot_store, this->plot_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::image: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Image> *>(
                obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->image_store, this->image_entry_event);
            break;
        }
        case MessageKind::entry + MessageKind::log: {
            auto &r = *static_cast<
                webcface::message::Entry<webcface::message::Log> *>(obj.get());
            this->logger_internal->debug("received {}", r);
            onRecvEntry(this, r, this->log_store, this->log_entry_event);
            break;
        }
        case MessageKind::func_info: {
            auto &r = *static_cast<webcface::message::FuncInfo *>(obj.get());
            this->logger_internal->debug("received {}", r);
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
        case MessageKind::plot:
        case MessageKind::image:
        case MessageKind::value + MessageKind::req:
        case MessageKind::text + MessageKind::req:
        case MessageKind::view + MessageKind::req:
        case MessageKind::canvas3d + MessageKind::req:
        case MessageKind::canvas2d + MessageKind::req:
        case MessageKind::robot_model + MessageKind::req:
        case MessageKind::plot + MessageKind::req:
        case MessageKind::image + MessageKind::req:
        case MessageKind::ping_status_req:
        case MessageKind::log + MessageKind::req:
        case MessageKind::log_default:
        case MessageKind::log_req_default:
            if (!message_kind_warned[kind]) {
                logger_internal->warn("invalid message kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        case MessageKind::log_entry_default:
        case MessageKind::view_old + MessageKind::entry:
        case MessageKind::unknown:
            break;
        default:
            if (!message_kind_warned[kind]) {
                logger_internal->warn("unknown message kind {}", kind);
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

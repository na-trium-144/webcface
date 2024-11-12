#include "webcface/server/member_data.h"
#include "webcface/server/store.h"
#include "webcface/server/server.h"
#include "webcface/message/message.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <algorithm>

WEBCFACE_NS_BEGIN
namespace server {

constexpr char field_separator = '.';

void MemberData::onClose() {
    if (con == nullptr) {
        return;
    }
    con = nullptr;
    logger->info("connection closed");
    closing.store(true);
    for (const auto &pm : pending_calls) {
        store->findAndDo(pm.first, [&](auto cd) {
            for (const auto &pi : pm.second) {
                switch (pi.second) {
                case 2:
                    cd->send(message::packSingle(
                        message::CallResponse{{}, pi.first, pm.first, false}));
                    cd->logger->debug("pending call aborted, sending "
                                      "call_response (caller_id {})",
                                      pi.first);
                    break;
                case 1:
                    cd->send(message::packSingle(message::CallResult{
                        {},
                        pi.first,
                        pm.first,
                        true,
                        ValAdaptor{"member(\"" + this->name.u8String() +
                                   "\") Disconnected"}}));
                    cd->logger->debug("pending call aborted, sending "
                                      "call_result (caller_id {})",
                                      pi.first);
                    break;
                case 0:
                    break;
                default:
                    throw std::runtime_error("invalid pending_call");
                }
            }
        });
    }
    {
        std::lock_guard lock(this->image_m);
        this->image_cv.notify_all();
    }
    store->forEach([&](auto cd) {
        std::lock_guard lock_cd(cd->image_m);
        if (cd->image_req.count(this->name)) {
            cd->image_cv.notify_all();
        }
    });
    for (auto &v : image_convert_thread) {
        for (auto &v2 : v.second) {
            if (v2.second->joinable()) {
                v2.second->join();
            }
        }
    }
    logger->trace("image_convert_thread stopped");
}
void MemberData::send() {
    if (connected() && send_len > 0) {
        send(message::packDone(send_buffer, send_len));
    }
    send_buffer.str("");
    send_len = 0;
}
void MemberData::send(const std::string &msg) {
    if (connected()) {
        store->server->send(con, msg);
    }
}
void MemberData::onConnect() { logger->debug("websocket connected"); }

bool MemberData::hasReq(const SharedString &member) {
    return std::any_of(this->value_req[member].begin(),
                       this->value_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->text_req[member].begin(),
                       this->text_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->image_req[member].begin(),
                       this->image_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->robot_model_req[member].begin(),
                       this->robot_model_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->canvas3d_req[member].begin(),
                       this->canvas3d_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->canvas3d_old_req[member].begin(),
                       this->canvas3d_old_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->view_req[member].begin(),
                       this->view_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->view_old_req[member].begin(),
                       this->view_old_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->canvas2d_req[member].begin(),
                       this->canvas2d_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->canvas2d_old_req[member].begin(),
                       this->canvas2d_old_req[member].end(),
                       [](const auto &it) { return it.second > 0; });
}

std::pair<unsigned int, SharedString> findReqField(StrMap2<unsigned int> &req,
                                                   const SharedString &member,
                                                   const SharedString &field) {
    for (const auto &req_it : req[member]) {
        if (req_it.first == field) {
            return std::make_pair(req_it.second, nullptr);
        } else if (req_it.first.startsWith(field.u8String() +
                                           field_separator)) {
            return std::make_pair(
                req_it.second,
                SharedString::fromU8String(req_it.first.u8String().substr(
                    field.u8String().size() + 1)));
        }
    }
    return std::make_pair<unsigned int, SharedString>(0, nullptr);
}

void MemberData::sendPing() {
    last_send_ping = std::chrono::system_clock::now();
    last_ping_duration = std::nullopt;
    send(message::packSingle(message::Ping{}));
}
void MemberData::onRecv(const std::string &message) {
    static std::unordered_map<int, bool> message_kind_warned;
    namespace MessageKind = webcface::message::MessageKind;
    auto messages = webcface::message::unpack(message, this->logger);
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::ping: {
            this->last_ping_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - this->last_send_ping);
            logger->debug("ping {} ms", this->last_ping_duration->count());
            break;
        }
        case MessageKind::ping_status_req: {
            this->ping_status_req = true;
            logger->debug("ping_status_req");
            if (store->ping_status != nullptr) {
                this->pack(message::PingStatus{{}, store->ping_status});
                logger->trace("send ping_status");
            }
            break;
        }
        case MessageKind::sync_init: {
            auto &v = *static_cast<webcface::message::SyncInit *>(obj.get());
            this->name = v.member_name;
            auto member_id_before = this->member_id;
            auto clients_by_id = store->clientsByIdCopy();
            auto prev_cli_it = std::find_if(
                clients_by_id.begin(), clients_by_id.end(),
                [&](const auto &it) { return it.second->name == this->name; });
            if (prev_cli_it != clients_by_id.end() && !this->name.empty()) {
                // すでに同じ名前のクライアントがいたら & 名前が空でなければ
                // そのidを使う
                this->member_id = v.member_id = prev_cli_it->first;
            } else {
                // コンストラクタですでに一意のidが振られているはず
                v.member_id = this->member_id;
            }
            v.addr = this->remote_addr;
            this->init_data = v;
            this->sync_init = true;
            store->initClientId(this->member_id, con);
            if (this->name.empty()) {
                logger->debug("sync_init (no name)");
            } else {
                this->logger = std::make_shared<spdlog::logger>(
                    std::to_string(this->member_id) + "_" + this->name.decode(),
                    this->sink);
                this->logger->set_level(this->logger_level);
                this->logger->debug(
                    "sync_init name={}, member_id={} (before {})",
                    this->name.decode(), this->member_id, member_id_before);
                this->logger->info("successfully connected and initialized.");
                // 全クライアントに新しいMemberを通知
                store->forEach([&](auto cd) {
                    if (cd->member_id != this->member_id) {
                        cd->pack(v);
                        cd->logger->trace("send sync_init {} ({})",
                                          this->name.decode(), this->member_id);
                    }
                });
            }
            // 逆に新しいMemberに他の全Memberのentryを通知
            store->forEachWithName([&](auto cd) {
                if (cd->member_id != this->member_id) {
                    this->pack(cd->init_data);
                    logger->trace("send sync_init {} ({})", cd->name.decode(),
                                  cd->member_id);

                    for (const auto &f : cd->value) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Value>{
                                {}, cd->member_id, f.first});
                            logger->trace("send value_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->text) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Text>{
                                {}, cd->member_id, f.first});
                            logger->trace("send text_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->robot_model) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::RobotModel>{
                                {}, cd->member_id, f.first});
                            logger->trace(
                                "send robot_model_entry {} of member {}",
                                f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas3d) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Canvas3D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas3d_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                            this->pack(webcface::message::Entry<
                                       webcface::message::Canvas3DOld>{
                                {}, cd->member_id, f.first});
                            logger->trace(
                                "send canvas3d_old_entry {} of member {}",
                                f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas2d) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Canvas2D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas2d_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                            this->pack(webcface::message::Entry<
                                       webcface::message::Canvas2DOld>{
                                {}, cd->member_id, f.first});
                            logger->trace(
                                "send canvas2d_old_entry {} of member {}",
                                f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->view) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::View>{
                                {}, cd->member_id, f.first});
                            logger->trace("send view_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                            this->pack(webcface::message::Entry<
                                       webcface::message::ViewOld>{
                                {}, cd->member_id, f.first});
                            logger->trace("send view_entry_old {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->image) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Image>{
                                {}, cd->member_id, f.first});
                            logger->trace("send image_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->log) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Log>{
                                {}, cd->member_id, f.first});
                            logger->trace("send log_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                            // 古いクライアントのために古いLogEntryも送る
                            if (f.first == message::Log::defaultLogName()) {
                                this->pack(webcface::message::LogEntryDefault{
                                    {}, cd->member_id});
                                logger->trace(
                                    "send log_entry_default(obsolete) of "
                                    "member {}",
                                    cd->member_id);
                            }
                        }
                    }
                    for (const auto &f : cd->func) {
                        if (!f.first.startsWith(field_separator)) {
                            this->pack(*f.second);
                            logger->trace("send func_info {} of member {}",
                                          f.second->field.decode(),
                                          cd->member_id);
                        }
                    }
                }
            });
            logger->trace("send sync_init_end");
            this->pack(webcface::message::SyncInitEnd{{},
                                                      WEBCFACE_SERVER_NAME,
                                                      WEBCFACE_VERSION,
                                                      this->member_id,
                                                      store->hostname});
            break;
        }
        case MessageKind::sync: {
            auto &v = *static_cast<webcface::message::Sync *>(obj.get());
            v.member_id = this->member_id;
            logger->debug("sync");
            // 1つ以上リクエストしているクライアントにはsyncの情報を流す
            store->forEach([&](auto cd) {
                if (cd->hasReq(this->name)) {
                    cd->pack(v);
                    cd->logger->trace("send sync {}", this->member_id);
                }
            });
            break;
        }
        case MessageKind::call: {
            auto &v = *static_cast<webcface::message::Call *>(obj.get());
            v.caller_member_id = this->member_id;
            logger->debug(
                "call caller_id={}, target_id={}, field={}, with {} args",
                v.caller_id, v.target_member_id, v.field.decode(),
                v.args.size());
            // そのままターゲットのクライアントに送る
            store->findConnectedAndDo(
                v.target_member_id,
                [&](auto cd) {
                    cd->pack(v);
                    cd->pending_calls[this->member_id][v.caller_id] = 2;
                    cd->logger->trace("send call caller_id={}, target_id={}, "
                                      "field={}, with {} args",
                                      v.caller_id, v.target_member_id,
                                      v.field.decode(), v.args.size());
                },
                [&]() {
                    // 関数存在しないor切断されているときの処理
                    this->pack(webcface::message::CallResponse{
                        {}, v.caller_id, v.caller_member_id, false});
                    logger->debug("call target not found");
                });
            break;
        }
        case MessageKind::call_response: {
            auto &v =
                *static_cast<webcface::message::CallResponse *>(obj.get());
            logger->debug("call_response to (member_id {}, caller_id {}), {}",
                          v.caller_member_id, v.caller_id, v.started);
            this->pending_calls[v.caller_member_id][v.caller_id] = 1;
            // そのままcallerに送る
            store->findAndDo(v.caller_member_id, [&](auto cd) {
                cd->pack(v);
                cd->logger->trace(
                    "send call_response to (member_id {}, caller_id {}), {}",
                    v.caller_member_id, v.caller_id, v.started);
            });
            break;
        }
        case MessageKind::call_result: {
            auto &v = *static_cast<webcface::message::CallResult *>(obj.get());
            logger->debug(
                "call_result to (member_id {}, caller_id {}), {} as {}",
                v.caller_member_id, v.caller_id,
                static_cast<std::string>(v.result),
                valTypeStr(v.result.valType()));
            this->pending_calls[v.caller_member_id][v.caller_id] = 0;
            // そのままcallerに送る
            store->findAndDo(v.caller_member_id, [&](auto cd) {
                cd->pack(v);
                cd->logger->trace("send call_result to (member_id {}, "
                                  "caller_id {}), {} as {}",
                                  v.caller_member_id, v.caller_id,
                                  static_cast<std::string>(v.result),
                                  valTypeStr(v.result.valType()));
            });
            break;
        }
        case MessageKind::value: {
            auto &v = *static_cast<webcface::message::Value *>(obj.get());
            if (v.data->size() == 1) {
                logger->debug("value {} = {}", v.field.decode(), (*v.data)[0]);
            } else {
                logger->debug("value {} = (array length = {})",
                              v.field.decode(), v.data->size());
            }
            if (!this->value.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::message::Entry<webcface::message::Value>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send value_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->value[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto req_field =
                    findReqField(cd->value_req, this->name, v.field);
                auto &req_id = req_field.first;
                auto &sub_field = req_field.second;
                if (req_id > 0) {
                    cd->pack(webcface::message::Res<webcface::message::Value>(
                        req_id, sub_field, v.data));
                    cd->logger->trace("send value_res req_id={} + '{}'", req_id,
                                      sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::text: {
            auto &v = *static_cast<webcface::message::Text *>(obj.get());
            logger->debug("text {} = {}", v.field.decode(),
                          static_cast<std::string>(*v.data));
            if (!this->text.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::message::Entry<webcface::message::Text>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send text_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->text[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto req_field =
                    findReqField(cd->text_req, this->name, v.field);
                auto &req_id = req_field.first;
                auto &sub_field = req_field.second;
                if (req_id > 0) {
                    cd->pack(webcface::message::Res<webcface::message::Text>(
                        req_id, sub_field, v.data));
                    cd->logger->trace("send text_res {}, req_id={} + '{}'",
                                      static_cast<std::string>(*v.data), req_id,
                                      sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::robot_model: {
            auto &v = *static_cast<webcface::message::RobotModel *>(obj.get());
            logger->debug("robot model {}", v.field.decode());
            if (!this->robot_model.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::RobotModel>{
                            {}, this->member_id, v.field});
                        cd->logger->trace(
                            "send robot_model_entry {} of member {}",
                            v.field.decode(), this->member_id);
                    }
                });
            }
            this->robot_model[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto req_field =
                    findReqField(cd->robot_model_req, this->name, v.field);
                auto &req_id = req_field.first;
                auto &sub_field = req_field.second;
                if (req_id > 0) {
                    cd->pack(
                        webcface::message::Res<webcface::message::RobotModel>(
                            req_id, sub_field, v.data));
                    cd->logger->trace("send robot_model_res, req_id={} + '{}'",
                                      req_id, sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::view: {
            auto &v = *static_cast<webcface::message::View *>(obj.get());
            logger->debug("view {} diff={}, length={}", v.field.decode(),
                          v.data_diff.size(),
                          v.data_ids ? v.data_ids->size() : 0);
            if (!this->view.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::message::Entry<webcface::message::View>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send view_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                        cd->pack(webcface::message::Entry<
                                 webcface::message::ViewOld>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send view_old_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            auto &this_view = this->view[v.field];
            for (auto &d : v.data_diff) {
                if (!d.second) {
                    d.second = std::make_shared<message::ViewComponent>();
                }
                this_view.components[d.first] = d.second;
            }
            bool ids_changed = false;
            std::vector<SharedString> prev_data_ids;
            if (v.data_ids) {
                ids_changed = true;
                prev_data_ids = std::move(this_view.data_ids);
                this_view.data_ids = std::move(*v.data_ids);
            }
            std::map<std::string, std::shared_ptr<message::ViewComponent>>
                old_diff;
            for (std::size_t i = 0; i < this_view.data_ids.size(); i++) {
                if (v.data_diff.count(this_view.data_ids[i].u8String()) ||
                    (ids_changed &&
                     (prev_data_ids.size() <= i ||
                      prev_data_ids.at(i) != this_view.data_ids[i]))) {
                    old_diff[std::to_string(i)] =
                        v.data_diff[this_view.data_ids[i].u8String()];
                }
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                {
                    auto req_field =
                        findReqField(cd->view_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(webcface::message::Res<
                                 webcface::message::View>(
                            req_id, sub_field, v.data_diff,
                            ids_changed
                                ? std::make_optional<std::vector<SharedString>>(
                                      this_view.data_ids)
                                : std::nullopt));
                        cd->logger->trace("send view_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                    }
                }
                {
                    auto req_field =
                        findReqField(cd->view_old_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(
                            webcface::message::Res<webcface::message::ViewOld>(
                                req_id, sub_field, old_diff,
                                this_view.data_ids.size()));
                        cd->logger->trace("send view_old_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                    }
                }
            });
            break;
        }
        case MessageKind::view_old: {
            auto &v = *static_cast<webcface::message::ViewOld *>(obj.get());
            logger->debug("view_old {} diff={}, length={}", v.field.decode(),
                          v.data_diff.size(), v.length);
            if (!this->view.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::message::Entry<webcface::message::View>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send view_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                        cd->pack(webcface::message::Entry<
                                 webcface::message::ViewOld>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send view_old_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            std::unordered_map<int, int> idx_next;
            auto &this_view = this->view[v.field];
            std::map<std::string, std::shared_ptr<message::ViewComponent>>
                new_diff;
            for (auto &d : v.data_diff) {
                std::size_t old_index = std::atoi(d.first.c_str());
                if (!d.second) {
                    d.second = std::make_shared<message::ViewComponent>();
                }
                int idx = idx_next[d.second->type]++;
                std::string id =
                    std::to_string(d.second->type) + "." + std::to_string(idx);
                this_view.components[id] = d.second;
                while (this_view.data_ids.size() <= old_index) {
                    this_view.data_ids.push_back(SharedString());
                }
                this_view.data_ids[old_index] = SharedString::fromU8String(id);
                new_diff[id] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                {
                    auto req_field =
                        findReqField(cd->view_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(
                            webcface::message::Res<webcface::message::View>(
                                req_id, sub_field, new_diff,
                                this_view.data_ids));
                        cd->logger->trace("send view_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                    }
                }
                {
                    auto req_field =
                        findReqField(cd->view_old_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(
                            webcface::message::Res<webcface::message::ViewOld>(
                                req_id, sub_field, v.data_diff,
                                this_view.data_ids.size()));
                        cd->logger->trace("send view_old_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                    }
                }
            });
            break;
        }
        case MessageKind::canvas3d: {
            auto &v = *static_cast<webcface::message::Canvas3D *>(obj.get());
            logger->debug("canvas3d {} diff={}, length={}", v.field.decode(),
                          v.data_diff.size(),
                          v.data_ids ? v.data_ids->size() : 0);
            if (!this->canvas3d.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas3D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas3d_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas3DOld>{
                            {}, this->member_id, v.field});
                        cd->logger->trace(
                            "send canvas3d_old_entry {} of member {}",
                            v.field.decode(), this->member_id);
                    }
                });
            }
            auto &this_canvas = this->canvas3d[v.field];
            for (auto &d : v.data_diff) {
                if (!d.second) {
                    d.second = std::make_shared<message::Canvas3DComponent>();
                }
                this_canvas.components[d.first] = d.second;
            }
            bool ids_changed = false;
            std::vector<SharedString> prev_data_ids;
            if (v.data_ids) {
                ids_changed = true;
                prev_data_ids = std::move(this_canvas.data_ids);
                this_canvas.data_ids = std::move(*v.data_ids);
            }
            std::map<std::string, std::shared_ptr<message::Canvas3DComponent>>
                old_diff;
            for (std::size_t i = 0; i < this_canvas.data_ids.size(); i++) {
                if (v.data_diff.count(this_canvas.data_ids[i].u8String()) ||
                    (ids_changed &&
                     (prev_data_ids.size() <= i ||
                      prev_data_ids.at(i) != this_canvas.data_ids[i]))) {
                    old_diff[std::to_string(i)] =
                        v.data_diff[this_canvas.data_ids[i].u8String()];
                }
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                {
                    auto req_field =
                        findReqField(cd->canvas3d_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(webcface::message::Res<
                                 webcface::message::Canvas3D>(
                            req_id, sub_field, v.data_diff,
                            ids_changed
                                ? std::make_optional<std::vector<SharedString>>(
                                      this_canvas.data_ids)
                                : std::nullopt));
                        cd->logger->trace(
                            "send canvas3d_old_res req_id={} + '{}'", req_id,
                            sub_field.decode());
                    }
                }
                {
                    auto req_field =
                        findReqField(cd->canvas3d_old_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(webcface::message::Res<
                                 webcface::message::Canvas3DOld>(
                            req_id, sub_field, old_diff,
                            this_canvas.data_ids.size()));
                        cd->logger->trace(
                            "send canvas3d_old_res req_id={} + '{}'", req_id,
                            sub_field.decode());
                    }
                }
            });
            break;
        }
        case MessageKind::canvas3d_old: {
            auto &v = *static_cast<webcface::message::Canvas3DOld *>(obj.get());
            logger->debug("canvas3d {} diff={}, length={}", v.field.decode(),
                          v.data_diff.size(), v.length);
            if (!this->canvas3d.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas3D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas3d_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas3DOld>{
                            {}, this->member_id, v.field});
                        cd->logger->trace(
                            "send canvas3d_old_entry {} of member {}",
                            v.field.decode(), this->member_id);
                    }
                });
            }
            std::unordered_map<int, int> idx_next;
            auto &this_canvas = this->canvas3d[v.field];
            std::map<std::string, std::shared_ptr<message::Canvas3DComponent>>
                new_diff;
            for (auto &d : v.data_diff) {
                std::size_t old_index = std::atoi(d.first.c_str());
                if (!d.second) {
                    d.second = std::make_shared<message::Canvas3DComponent>();
                }
                int idx = idx_next[d.second->type]++;
                std::string id =
                    std::to_string(d.second->type) + "." + std::to_string(idx);
                this_canvas.components[id] = d.second;
                while (this_canvas.data_ids.size() <= old_index) {
                    this_canvas.data_ids.push_back(SharedString());
                }
                this_canvas.data_ids[old_index] =
                    SharedString::fromU8String(id);
                new_diff[id] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                {
                    auto req_field =
                        findReqField(cd->canvas3d_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(
                            webcface::message::Res<webcface::message::Canvas3D>(
                                req_id, sub_field, new_diff,
                                this_canvas.data_ids));
                        cd->logger->trace("send canvas3d_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                    }
                }
                {
                    auto req_field =
                        findReqField(cd->canvas3d_old_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(webcface::message::Res<
                                 webcface::message::Canvas3DOld>(
                            req_id, sub_field, v.data_diff,
                            this_canvas.data_ids.size()));
                        cd->logger->trace(
                            "send canvas2d_old_res req_id={} + '{}'", req_id,
                            sub_field.decode());
                    }
                }
            });
            break;
        }
        case MessageKind::canvas2d: {
            auto &v = *static_cast<webcface::message::Canvas2D *>(obj.get());
            logger->debug("canvas2d {} diff={}, length={}", v.field.decode(),
                          v.data_diff.size(),
                          v.data_ids ? v.data_ids->size() : 0);
            if (!this->canvas2d.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas2D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas2d_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas2DOld>{
                            {}, this->member_id, v.field});
                        cd->logger->trace(
                            "send canvas2d_old_entry {} of member {}",
                            v.field.decode(), this->member_id);
                    }
                });
            }
            auto &this_canvas = this->canvas2d[v.field];
            this_canvas.width = v.width;
            this_canvas.height = v.height;
            for (auto &d : v.data_diff) {
                if (!d.second) {
                    d.second = std::make_shared<message::Canvas2DComponent>();
                }
                this_canvas.components[d.first] = d.second;
            }
            bool ids_changed = false;
            std::vector<SharedString> prev_data_ids;
            if (v.data_ids) {
                ids_changed = true;
                prev_data_ids = std::move(this_canvas.data_ids);
                this_canvas.data_ids = std::move(*v.data_ids);
            }
            std::map<std::string, std::shared_ptr<message::Canvas2DComponent>>
                old_diff;
            for (std::size_t i = 0; i < this_canvas.data_ids.size(); i++) {
                if (v.data_diff.count(this_canvas.data_ids[i].u8String()) ||
                    (ids_changed &&
                     (prev_data_ids.size() <= i ||
                      prev_data_ids.at(i) != this_canvas.data_ids[i]))) {
                    old_diff[std::to_string(i)] =
                        v.data_diff[this_canvas.data_ids[i].u8String()];
                }
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                {
                    auto req_field =
                        findReqField(cd->canvas2d_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(webcface::message::Res<
                                 webcface::message::Canvas2D>(
                            req_id, sub_field, v.width, v.height, v.data_diff,
                            ids_changed
                                ? std::make_optional<std::vector<SharedString>>(
                                      this_canvas.data_ids)
                                : std::nullopt));
                        cd->logger->trace("send canvas2d_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                    }
                }
                {
                    auto req_field =
                        findReqField(cd->canvas2d_old_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(webcface::message::Res<
                                 webcface::message::Canvas2DOld>(
                            req_id, sub_field, v.width, v.height, old_diff,
                            this_canvas.data_ids.size()));
                        cd->logger->trace(
                            "send canvas2d_old_res req_id={} + '{}'", req_id,
                            sub_field.decode());
                    }
                }
            });
            break;
        }
        case MessageKind::canvas2d_old: {
            auto &v = *static_cast<webcface::message::Canvas2DOld *>(obj.get());
            logger->debug("canvas2d {} diff={}, length={}", v.field.decode(),
                          v.data_diff.size(), v.length);
            if (!this->canvas2d.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas2D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas2d_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas2DOld>{
                            {}, this->member_id, v.field});
                        cd->logger->trace(
                            "send canvas2d_old_entry {} of member {}",
                            v.field.decode(), this->member_id);
                    }
                });
            }
            std::unordered_map<int, int> idx_next;
            auto &this_canvas = this->canvas2d[v.field];
            this_canvas.width = v.width;
            this_canvas.height = v.height;
            std::map<std::string, std::shared_ptr<message::Canvas2DComponent>>
                new_diff;
            for (auto &d : v.data_diff) {
                std::size_t old_index = std::atoi(d.first.c_str());
                if (!d.second) {
                    d.second = std::make_shared<message::Canvas2DComponent>();
                }
                int idx = idx_next[d.second->type]++;
                std::string id =
                    std::to_string(d.second->type) + "." + std::to_string(idx);
                this_canvas.components[id] = d.second;
                while (this_canvas.data_ids.size() <= old_index) {
                    this_canvas.data_ids.push_back(SharedString());
                }
                this_canvas.data_ids[old_index] =
                    SharedString::fromU8String(id);
                new_diff[id] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                {
                    auto req_field =
                        findReqField(cd->canvas2d_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(
                            webcface::message::Res<webcface::message::Canvas2D>(
                                req_id, sub_field, v.width, v.height, new_diff,
                                this_canvas.data_ids));
                        cd->logger->trace("send canvas2d_res req_id={} + '{}'",
                                          req_id, sub_field.decode());
                    }
                }
                {
                    auto req_field =
                        findReqField(cd->canvas2d_old_req, this->name, v.field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    if (req_id > 0) {
                        cd->pack(webcface::message::Res<
                                 webcface::message::Canvas2DOld>(
                            req_id, sub_field, v.width, v.height, v.data_diff,
                            this_canvas.data_ids.size()));
                        cd->logger->trace(
                            "send canvas2d_old_res req_id={} + '{}'", req_id,
                            sub_field.decode());
                    }
                }
            });
            break;
        }
        case MessageKind::image: {
            auto &v = *static_cast<webcface::message::Image *>(obj.get());
            logger->debug("image {} ({} x {})", v.field.decode(), v.width_,
                          v.height_);
            if (!this->image.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::message::Entry<webcface::message::Image>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send image_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            // このimageをsubscribeしてるところに送り返す
            {
                std::vector<std::unique_ptr<std::lock_guard<std::mutex>>> locks;
                store->forEach([&](auto cd) {
                    locks.emplace_back(
                        std::make_unique<std::lock_guard<std::mutex>>(
                            cd->image_m));
                });
                this->image[v.field] = v;
                this->image_changed[v.field]++;
                store->forEach([&](auto cd) {
                    if (cd->image_req.count(this->name)) {
                        cd->image_cv.notify_all();
                    }
                });
            }
            break;
        }
        case MessageKind::log:
        case MessageKind::log_default: {
            SharedString field;
            std::shared_ptr<std::deque<message::LogLine>> log_data;
            if (kind == MessageKind::log) {
                auto &v = *static_cast<webcface::message::Log *>(obj.get());
                logger->debug("log {}: {} lines", v.field.decode(),
                              v.log->size());
                field = v.field;
                log_data = v.log;
            } else {
                auto &v =
                    *static_cast<webcface::message::LogDefault *>(obj.get());
                logger->debug("log_default(obsolete) {} lines", v.log->size());
                field = message::Log::defaultLogName();
                log_data = v.log;
            }
            if (!this->log.count(field)) {
                if (store->keep_log >= 0 &&
                    log_data->size() >
                        static_cast<unsigned int>(store->keep_log)) {
                    logger->info(
                        "number of log lines reached {}, so the oldest "
                        "log will be romoved.",
                        store->keep_log);
                }
                while (store->keep_log >= 0 &&
                       log_data->size() >
                           static_cast<unsigned int>(store->keep_log)) {
                    log_data->pop_front();
                }
                this->log.emplace(field, log_data);
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<message::Log>{
                            {}, this->member_id, field});
                        cd->logger->trace("send log_entry {} of member {}",
                                          field.decode(), this->member_id);
                        // 古いクライアントのために古いLogEntryも送る
                        if (field == message::Log::defaultLogName()) {
                            cd->pack(webcface::message::LogEntryDefault{
                                {}, this->member_id});
                            cd->logger->trace(
                                "send log_entry_default(obsolete) of member {}",
                                this->member_id);
                        }
                    }
                });
            } else {
                auto this_log = this->log.at(field);
                if (store->keep_log >= 0 &&
                    this_log->size() <
                        static_cast<unsigned int>(store->keep_log) &&
                    this_log->size() + log_data->size() >=
                        static_cast<unsigned int>(store->keep_log)) {
                    logger->info(
                        "number of log lines reached {}, so the oldest "
                        "log will be romoved.",
                        store->keep_log);
                }
                for (auto &ll : *log_data) {
                    this_log->push_back(ll);
                }
                while (store->keep_log >= 0 &&
                       this_log->size() >
                           static_cast<unsigned int>(store->keep_log)) {
                    this_log->pop_front();
                }
            }
            // このlogをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto req_field = findReqField(cd->log_req, this->name, field);
                auto &req_id = req_field.first;
                auto &sub_field = req_field.second;
                if (req_id > 0) {
                    cd->pack(webcface::message::Res<webcface::message::Log>(
                        req_id, sub_field, log_data));
                    cd->logger->trace("send log_res req_id={} + '{}'", req_id,
                                      sub_field.decode());
                }
                if (cd->log_req_default.count(this->name)) {
                    cd->pack(message::LogDefault{this->member_id, log_data});
                    cd->logger->trace("send log_default(obsolete) {} lines",
                                      log_data->size());
                }
            });
            break;
        }
        case MessageKind::func_info: {
            auto &v = *static_cast<webcface::message::FuncInfo *>(obj.get());
            v.member_id = this->member_id;
            logger->debug("func_info {}", v.field.decode());
            if (!this->func.count(v.field) &&
                !v.field.startsWith(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->member_id != this->member_id) {
                        cd->pack(v);
                        cd->logger->trace("send func_info {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->func[v.field] = std::make_shared<message::FuncInfo>(v);
            break;
        }
        case MessageKind::req + MessageKind::value: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::Value> *>(obj.get());
            logger->debug("request value ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->value) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::message::Res<webcface::message::Value>{
                                s.req_id, sub_field, it.second});
                        logger->trace("send value_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            value_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::text: {
            auto &s =
                *static_cast<webcface::message::Req<webcface::message::Text> *>(
                    obj.get());
            logger->debug("request text ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->text) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::message::Res<webcface::message::Text>{
                                s.req_id, sub_field, it.second});
                        logger->trace("send text_res {}, req_id={} + '{}'",
                                      static_cast<std::string>(*it.second),
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            text_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::robot_model: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::RobotModel> *>(
                obj.get());
            logger->debug("request robot_model ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->robot_model) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        this->pack(webcface::message::Res<
                                   webcface::message::RobotModel>{
                            s.req_id, sub_field, it.second});
                        logger->trace("send robot_model_res, req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            robot_model_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::view: {
            auto &s =
                *static_cast<webcface::message::Req<webcface::message::View> *>(
                    obj.get());
            logger->debug("request view ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->view) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::message::Res<webcface::message::View>{
                                s.req_id, sub_field, it.second.components,
                                it.second.data_ids});
                        logger->trace("send view_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            view_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::view_old: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::ViewOld> *>(
                obj.get());
            logger->debug("request view_old ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->view) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        std::map<std::string,
                                 std::shared_ptr<message::ViewComponent>>
                            old_components;
                        for (std::size_t i = 0; i < it.second.data_ids.size();
                             i++) {
                            old_components[std::to_string(i)] =
                                it.second.components.at(
                                    it.second.data_ids[i].u8String());
                        }
                        this->pack(
                            webcface::message::Res<webcface::message::ViewOld>{
                                s.req_id, sub_field, old_components,
                                old_components.size()});
                        logger->trace("send view_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            view_old_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::canvas3d: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::Canvas3D> *>(
                obj.get());
            logger->debug("request canvas3d ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas3d) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::message::Res<webcface::message::Canvas3D>{
                                s.req_id, sub_field, it.second.components,
                                it.second.data_ids});
                        logger->trace("send canvas3d_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            canvas3d_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::canvas3d_old: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::Canvas3DOld> *>(
                obj.get());
            logger->debug("request canvas3d ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas3d) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        std::map<std::string,
                                 std::shared_ptr<message::Canvas3DComponent>>
                            old_components;
                        for (std::size_t i = 0; i < it.second.data_ids.size();
                             i++) {
                            old_components[std::to_string(i)] =
                                it.second.components.at(
                                    it.second.data_ids[i].u8String());
                        }
                        this->pack(webcface::message::Res<
                                   webcface::message::Canvas3DOld>{
                            s.req_id, sub_field, old_components,
                            old_components.size()});
                        logger->trace("send canvas3d_old_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            canvas3d_old_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::canvas2d: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::Canvas2D> *>(
                obj.get());
            logger->debug("request canvas2d ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas2d) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::message::Res<webcface::message::Canvas2D>{
                                s.req_id, sub_field, it.second.width,
                                it.second.height, it.second.components,
                                it.second.data_ids});
                        logger->trace("send canvas2d_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            canvas2d_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::canvas2d_old: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::Canvas2DOld> *>(
                obj.get());
            logger->debug("request canvas2d_olf ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas2d) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        std::map<std::string,
                                 std::shared_ptr<message::Canvas2DComponent>>
                            old_components;
                        for (std::size_t i = 0; i < it.second.data_ids.size();
                             i++) {
                            old_components[std::to_string(i)] =
                                it.second.components.at(
                                    it.second.data_ids[i].u8String());
                        }

                        this->pack(webcface::message::Res<
                                   webcface::message::Canvas2DOld>{
                            s.req_id, sub_field, it.second.width,
                            it.second.height, old_components,
                            old_components.size()});
                        logger->trace("send canvas2d_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            canvas2d_old_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::image: {
            auto &s = *static_cast<
                webcface::message::Req<webcface::message::Image> *>(obj.get());
            logger->debug("request image ({}): {} from {}, {} x {}, color={}, "
                          "mode={}, q={}, fps={}",
                          s.req_id, s.field.decode(), s.member.decode(),
                          s.rows.value_or(-1), s.cols.value_or(-1),
                          (s.color_mode ? static_cast<int>(*s.color_mode) : -1),
                          static_cast<int>(s.cmp_mode), s.quality,
                          s.frame_rate.value_or(-1));
            {
                std::lock_guard lock(this->image_m);
                image_req_info[s.member][s.field] = s;
                image_req[s.member][s.field] = s.req_id;
                image_req_changed[s.member][s.field]++;
                this->image_cv.notify_all();
                if (!image_convert_thread[s.member].count(s.field)) {
                    this->image_convert_thread[s.member].emplace(
                        s.field,
                        std::thread([this, member = s.member, field = s.field] {
                            this->imageConvertThreadMain(member, field);
                        }));
                }
            }
            break;
        }
        case MessageKind::req + MessageKind::log: {
            auto &s =
                *static_cast<webcface::message::Req<webcface::message::Log> *>(
                    obj.get());
            logger->debug("request log ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                // if (!this->hasReq(s.member)) {
                //     this->pack(webcface::message::Sync{cd->member_id,
                //                                        cd->last_sync_time});
                //     logger->trace("send sync {}", this->member_id);
                // }
                for (const auto &it : cd->log) {
                    if (it.first == s.field ||
                        it.first.startsWith(s.field.u8String() +
                                            field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString::fromU8String(
                                it.first.u8String().substr(
                                    s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::message::Res<webcface::message::Log>{
                                s.req_id, sub_field, it.second});
                        logger->trace("send log_res {} lines, req_id={} + '{}'",
                                      it.second->size(), s.req_id,
                                      sub_field.decode());
                    }
                }
            });
            log_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::log_req_default: {
            auto &s =
                *static_cast<webcface::message::LogReqDefault *>(obj.get());
            logger->debug("request log_default(obsolete) from {}",
                          s.member.decode());
            this->log_req_default.insert(s.member);
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (cd->log.count(message::Log::defaultLogName())) {
                    auto log_data = cd->log.at(message::Log::defaultLogName());
                    this->pack(
                        webcface::message::LogDefault{cd->member_id, log_data});
                    logger->trace("send log_default(obsolete) {} lines",
                                  log_data->size());
                }
            });
            break;
        }
        case MessageKind::entry + MessageKind::value:
        case MessageKind::res + MessageKind::value:
        case MessageKind::entry + MessageKind::text:
        case MessageKind::res + MessageKind::text:
        case MessageKind::entry + MessageKind::robot_model:
        case MessageKind::res + MessageKind::robot_model:
        case MessageKind::entry + MessageKind::view:
        case MessageKind::res + MessageKind::view:
        case MessageKind::entry + MessageKind::view_old:
        case MessageKind::res + MessageKind::view_old:
        case MessageKind::entry + MessageKind::canvas3d:
        case MessageKind::res + MessageKind::canvas3d:
        case MessageKind::entry + MessageKind::canvas3d_old:
        case MessageKind::res + MessageKind::canvas3d_old:
        case MessageKind::entry + MessageKind::canvas2d:
        case MessageKind::res + MessageKind::canvas2d:
        case MessageKind::entry + MessageKind::canvas2d_old:
        case MessageKind::res + MessageKind::canvas2d_old:
        case MessageKind::entry + MessageKind::image:
        case MessageKind::res + MessageKind::image:
        case MessageKind::entry + MessageKind::log:
        case MessageKind::res + MessageKind::log:
        case MessageKind::log_entry_default:
        case MessageKind::sync_init_end:
        case MessageKind::ping_status:
            if (!message_kind_warned[kind]) {
                logger->warn("Invalid message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        default:
            if (!message_kind_warned[kind]) {
                logger->warn("Unknown message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        }
    }
    store->clientSendAll();
}

} // namespace server
WEBCFACE_NS_END

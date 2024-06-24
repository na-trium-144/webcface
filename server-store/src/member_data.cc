#include "webcface/server/member_data.h"
#include "webcface/server/store.h"
#include <webcface/server/server.h>
#include "webcface/message/message.h"
#include <webcface/common/def.h>
#include <algorithm>
#include <Magick++.h>

WEBCFACE_NS_BEGIN
namespace Server {

constexpr char8_t field_separator = u8'.';

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
                    cd->send(Message::packSingle(
                        Message::CallResponse{{}, pi.first, pm.first, false}));
                    cd->logger->debug("pending call aborted, sending "
                                      "call_response (caller_id {})",
                                      pi.first);
                    break;
                case 1:
                    cd->send(Message::packSingle(Message::CallResult{
                        {},
                        pi.first,
                        pm.first,
                        true,
                        ValAdaptor{u8"member(\"" + this->name.u8String() +
                                   u8"\") Disconnected"}}));
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
    for (auto &v : image_convert_thread) {
        for (auto &v2 : v.second) {
            v2.second->join();
        }
    }
    for (auto &v : image_cv) {
        std::lock_guard lock(image_m[v.first]);
        v.second.notify_all();
    }
    logger->trace("image_convert_thread stopped");
}
void MemberData::send() {
    if (connected() && send_len > 0) {
        send(Message::packDone(send_buffer, send_len));
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
           std::any_of(this->view_req[member].begin(),
                       this->view_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->canvas2d_req[member].begin(),
                       this->canvas2d_req[member].end(),
                       [](const auto &it) { return it.second > 0; });
}

std::pair<unsigned int, SharedString> findReqField(StrMap2<unsigned int> &req,
                                                   const SharedString &member,
                                                   const SharedString &field) {
    for (const auto &req_it : req[member]) {
        if (req_it.first == field) {
            return std::make_pair(req_it.second, nullptr);
        } else if (req_it.first.u8String().starts_with(field.u8String() +
                                                       field_separator)) {
            return std::make_pair(req_it.second,
                                  SharedString(req_it.first.u8String().substr(
                                      field.u8String().size() + 1)));
        }
    }
    return std::make_pair<unsigned int, SharedString>(0, nullptr);
}

void MemberData::sendPing() {
    last_send_ping = std::chrono::system_clock::now();
    last_ping_duration = std::nullopt;
    send(Message::packSingle(Message::Ping{}));
}
void MemberData::onRecv(const std::string &message) {
    static std::unordered_map<int, bool> message_kind_warned;
    namespace MessageKind = webcface::Message::MessageKind;
    auto messages = webcface::Message::unpack(message, this->logger);
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
                this->pack(Message::PingStatus{{}, store->ping_status});
                logger->trace("send ping_status");
            }
            break;
        }
        case MessageKind::sync_init: {
            auto v = std::any_cast<webcface::Message::SyncInit>(obj);
            this->name = v.member_name;
            auto member_id_before = this->member_id;
            auto prev_cli_it = std::find_if(
                store->clients_by_id.begin(), store->clients_by_id.end(),
                [&](const auto &it) { return it.second->name == this->name; });
            if (prev_cli_it != store->clients_by_id.end()) {
                this->member_id = v.member_id = prev_cli_it->first;
            } else {
                // コンストラクタですでに一意のidが振られているはず
                v.member_id = this->member_id;
            }
            v.addr = this->remote_addr;
            this->init_data = v;
            this->sync_init = true;
            store->clients_by_id.erase(this->member_id);
            store->clients_by_id.emplace(this->member_id,
                                         store->getClient(con));
            if (this->name.empty()) {
                logger->debug("sync_init (no name)");
            } else {
                this->logger = std::make_shared<spdlog::logger>(
                    std::to_string(this->member_id) + "_" + this->name_s,
                    this->sink);
                this->logger->set_level(this->logger_level);
                this->logger->debug(
                    "sync_init name={}, member_id={} (before {})", this->name_s,
                    this->member_id, member_id_before);
                this->logger->info("successfully connected and initialized.");
                // 全クライアントに新しいMemberを通知
                store->forEach([&](auto cd) {
                    if (cd->member_id != this->member_id) {
                        cd->pack(v);
                        cd->logger->trace("send sync_init {} ({})",
                                          this->name_s, this->member_id);
                    }
                });
            }
            this->pack(webcface::Message::SvrVersion{
                {}, WEBCFACE_SERVER_NAME, WEBCFACE_VERSION});
            // 逆に新しいMemberに他の全Memberのentryを通知
            store->forEachWithName([&](auto cd) {
                if (cd->member_id != this->member_id) {
                    this->pack(cd->init_data);
                    logger->trace("send sync_init {} ({})", cd->name_s,
                                  cd->member_id);

                    for (const auto &f : cd->value) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Value>{
                                {}, cd->member_id, f.first});
                            logger->trace("send value_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->text) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Text>{
                                {}, cd->member_id, f.first});
                            logger->trace("send text_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->robot_model) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::RobotModel>{
                                {}, cd->member_id, f.first});
                            logger->trace(
                                "send robot_model_entry {} of member {}",
                                f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas3d) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Canvas3D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas3d_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas2d) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Canvas2D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas2d_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->view) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::View>{
                                {}, cd->member_id, f.first});
                            logger->trace("send view_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->image) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Image>{
                                {}, cd->member_id, f.first});
                            logger->trace("send image_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->func) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(*f.second);
                            logger->trace("send func_info {} of member {}",
                                          f.second->field.decode(),
                                          cd->member_id);
                        }
                    }
                }
            });
            break;
        }
        case MessageKind::sync: {
            auto v = std::any_cast<webcface::Message::Sync>(obj);
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
            auto v = std::any_cast<webcface::Message::Call>(obj);
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
                    this->pack(webcface::Message::CallResponse{
                        {}, v.caller_id, v.caller_member_id, false});
                    logger->debug("call target not found");
                });
            break;
        }
        case MessageKind::call_response: {
            auto v = std::any_cast<webcface::Message::CallResponse>(obj);
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
            auto v = std::any_cast<webcface::Message::CallResult>(obj);
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
            auto v = std::any_cast<webcface::Message::Value>(obj);
            if (v.data->size() == 1) {
                logger->debug("value {} = {}", v.field.decode(), (*v.data)[0]);
            } else {
                logger->debug("value {} = (array length = {})",
                              v.field.decode(), v.data->size());
            }
            if (!this->value.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::Value>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send value_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->value[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->value_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(webcface::Message::Res<webcface::Message::Value>(
                        req_id, sub_field, v.data));
                    cd->logger->trace("send value_res req_id={} + '{}'", req_id,
                                      sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::text: {
            auto v = std::any_cast<webcface::Message::Text>(obj);
            logger->debug("text {} = {}", v.field.decode(),
                          static_cast<std::string>(*v.data));
            if (!this->text.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::Text>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send text_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->text[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->text_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(webcface::Message::Res<webcface::Message::Text>(
                        req_id, sub_field, v.data));
                    cd->logger->trace("send text_res {}, req_id={} + '{}'",
                                      static_cast<std::string>(*v.data), req_id,
                                      sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::robot_model: {
            auto v = std::any_cast<webcface::Message::RobotModel>(obj);
            logger->debug("robot model {}", v.field.decode());
            if (!this->robot_model.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::Message::Entry<
                                 webcface::Message::RobotModel>{
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
                auto [req_id, sub_field] =
                    findReqField(cd->robot_model_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(
                        webcface::Message::Res<webcface::Message::RobotModel>(
                            req_id, sub_field, v.data));
                    cd->logger->trace("send robot_model_res, req_id={} + '{}'",
                                      req_id, sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::view: {
            auto v = std::any_cast<webcface::Message::View>(obj);
            logger->debug("view {} diff={}, length={}", v.field.decode(),
                          v.data_diff->size(), v.length);
            if (!this->view.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::View>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send view_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->view[v.field].resize(v.length);
            for (auto &d : *v.data_diff) {
                this->view[v.field][std::stoi(d.first)] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->view_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(webcface::Message::Res<webcface::Message::View>(
                        req_id, sub_field, v.data_diff, v.length));
                    cd->logger->trace("send view_res req_id={} + '{}'", req_id,
                                      sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::canvas3d: {
            auto v = std::any_cast<webcface::Message::Canvas3D>(obj);
            logger->debug("canvas3d {} diff={}, length={}", v.field.decode(),
                          v.data_diff->size(), v.length);
            if (!this->canvas3d.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::Message::Entry<
                                 webcface::Message::Canvas3D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas3d_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->canvas3d[v.field].resize(v.length);
            for (const auto &d : *v.data_diff) {
                this->canvas3d[v.field][std::stoi(d.first)] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->canvas3d_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(
                        webcface::Message::Res<webcface::Message::Canvas3D>(
                            req_id, sub_field, v.data_diff, v.length));
                    cd->logger->trace("send canvas3d_res req_id={} + '{}'",
                                      req_id, sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::canvas2d: {
            auto v = std::any_cast<webcface::Message::Canvas2D>(obj);
            logger->debug("canvas2d {} diff={}, length={}", v.field.decode(),
                          v.data_diff->size(), v.length);
            if (!this->canvas2d.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::Message::Entry<
                                 webcface::Message::Canvas2D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas2d_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->canvas2d[v.field].width = v.width;
            this->canvas2d[v.field].height = v.height;
            this->canvas2d[v.field].components.resize(v.length);
            for (auto &d : *v.data_diff) {
                this->canvas2d[v.field].components[std::stoi(d.first)] =
                    d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->canvas2d_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(
                        webcface::Message::Res<webcface::Message::Canvas2D>(
                            req_id, sub_field, v.width, v.height, v.data_diff,
                            v.length));
                    cd->logger->trace("send canvas2d_res req_id={} + '{}'",
                                      req_id, sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::image: {
            auto v = std::any_cast<webcface::Message::Image>(obj);
            logger->debug("image {} ({} x {})", v.field.decode(), v.width_,
                          v.height_);
            if (!this->image.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::Image>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send image_entry {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            // このimageをsubscribeしてるところに送り返す
            {
                std::lock_guard lock(this->image_m[v.field]);
                this->image[v.field] = v;
                this->image_changed[v.field]++;
                this->image_cv[v.field].notify_all();
            }
            break;
        }
        case MessageKind::log: {
            auto v = std::any_cast<webcface::Message::Log>(obj);
            v.member_id = this->member_id;
            logger->debug("log {} lines", v.log->size());
            if (store->keep_log >= 0 &&
                this->log->size() <
                    static_cast<unsigned int>(store->keep_log) &&
                this->log->size() + v.log->size() >=
                    static_cast<unsigned int>(store->keep_log)) {
                logger->info("number of log lines reached {}, so the oldest "
                             "log will be romoved.",
                             store->keep_log);
            }
            for (auto &ll : *v.log) {
                this->log->push_back(ll);
            }
            while (store->keep_log >= 0 &&
                   this->log->size() >
                       static_cast<unsigned int>(store->keep_log)) {
                this->log->pop_front();
            }
            // このlogをsubscribeしてるところに送り返す
            store->forEach([&](auto cd) {
                if (cd->log_req.count(this->name)) {
                    cd->pack(v);
                    cd->logger->trace("send log {} lines", v.log->size());
                }
            });
            break;
        }
        case MessageKind::func_info: {
            auto v = std::any_cast<webcface::Message::FuncInfo>(obj);
            v.member_id = this->member_id;
            logger->debug("func_info {}", v.field.decode());
            if (!this->func.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->member_id != this->member_id) {
                        cd->pack(v);
                        cd->logger->trace("send func_info {} of member {}",
                                          v.field.decode(), this->member_id);
                    }
                });
            }
            this->func[v.field] = std::make_shared<Message::FuncInfo>(v);
            break;
        }
        case MessageKind::req + MessageKind::value: {
            auto s =
                std::any_cast<webcface::Message::Req<webcface::Message::Value>>(
                    obj);
            logger->debug("request value ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->value) {
                    if (it.first == s.field ||
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
                                s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Value>{
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
            auto s =
                std::any_cast<webcface::Message::Req<webcface::Message::Text>>(
                    obj);
            logger->debug("request text ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->text) {
                    if (it.first == s.field ||
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
                                s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Text>{
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
            auto s = std::any_cast<
                webcface::Message::Req<webcface::Message::RobotModel>>(obj);
            logger->debug("request robot_model ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->robot_model) {
                    if (it.first == s.field ||
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
                                s.field.u8String().size() + 1));
                        }
                        this->pack(webcface::Message::Res<
                                   webcface::Message::RobotModel>{
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
            auto s =
                std::any_cast<webcface::Message::Req<webcface::Message::View>>(
                    obj);
            logger->debug("request view ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->view) {
                    if (it.first == s.field ||
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string, webcface::Message::ViewComponent>>();
                        for (std::size_t i = 0; i < it.second.size(); i++) {
                            diff->emplace(std::to_string(i), it.second[i]);
                        }
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
                                s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::View>{
                                s.req_id, sub_field, diff, it.second.size()});
                        logger->trace("send view_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            view_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::canvas3d: {
            auto s = std::any_cast<
                webcface::Message::Req<webcface::Message::Canvas3D>>(obj);
            logger->debug("request canvas3d ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas3d) {
                    if (it.first == s.field ||
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            webcface::Message::Canvas3DComponent>>();
                        for (std::size_t i = 0; i < it.second.size(); i++) {
                            diff->emplace(std::to_string(i), it.second[i]);
                        }
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
                                s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Canvas3D>{
                                s.req_id, sub_field, diff, it.second.size()});
                        logger->trace("send canvas3d_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            canvas3d_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::canvas2d: {
            auto s = std::any_cast<
                webcface::Message::Req<webcface::Message::Canvas2D>>(obj);
            logger->debug("request canvas2d ({}): {} from {}", s.req_id,
                          s.field.decode(), s.member.decode());
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas2d) {
                    if (it.first == s.field ||
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            webcface::Message::Canvas2DComponent>>();
                        for (std::size_t i = 0; i < it.second.components.size();
                             i++) {
                            diff->emplace(std::to_string(i),
                                          it.second.components[i]);
                        }
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
                                s.field.u8String().size() + 1));
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Canvas2D>{
                                s.req_id, sub_field, it.second.width,
                                it.second.height, diff,
                                it.second.components.size()});
                        logger->trace("send canvas2d_res req_id={} + '{}'",
                                      s.req_id, sub_field.decode());
                    }
                }
            });
            canvas2d_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::image: {
            auto s =
                std::any_cast<webcface::Message::Req<webcface::Message::Image>>(
                    obj);
            logger->debug("request image ({}): {} from {}, {} x {}, color={}, "
                          "mode={}, q={}, fps={}",
                          s.req_id, s.field.decode(), s.member.decode(),
                          s.rows.value_or(-1), s.cols.value_or(-1),
                          (s.color_mode ? static_cast<int>(*s.color_mode) : -1),
                          static_cast<int>(s.cmp_mode), s.quality,
                          s.frame_rate.value_or(-1));
            image_req_info[s.member][s.field] = s;
            image_req[s.member][s.field] = s.req_id;
            image_req_changed[s.member][s.field]++;
            if (!image_convert_thread[s.member].count(s.field)) {
                image_convert_thread[s.member].emplace(
                    s.field,
                    std::thread([this, member = s.member, field = s.field] {
                        this->imageConvertThreadMain(member, field);
                    }));
            }
            break;
        }
        case MessageKind::log_req: {
            auto s = std::any_cast<webcface::Message::LogReq>(obj);
            logger->debug("request log from {}", s.member.decode());
            log_req.insert(s.member);
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                this->pack(webcface::Message::Log{cd->member_id, cd->log});
                logger->trace("send log {} lines", cd->log->size());
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
        case MessageKind::entry + MessageKind::canvas3d:
        case MessageKind::res + MessageKind::canvas3d:
        case MessageKind::entry + MessageKind::canvas2d:
        case MessageKind::res + MessageKind::canvas2d:
        case MessageKind::entry + MessageKind::image:
        case MessageKind::res + MessageKind::image:
        case MessageKind::svr_version:
        case MessageKind::ping_status:
            if (!message_kind_warned[kind]) {
                logger->warn("Invalid Message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        default:
            if (!message_kind_warned[kind]) {
                logger->warn("Unknown Message Kind {}", kind);
                message_kind_warned[kind] = true;
            }
            break;
        }
    }
    store->clientSendAll();
}

static std::string magickColorMap(int mode) {
    switch (mode) {
    case 0: // ImageColorMode::gray:
        return "K";
    case 1: // ImageColorMode::bgr:
        return "BGR";
    case 2: // ImageColorMode::rgb:
        return "RGB";
    case 3: // ImageColorMode::bgra:
        return "BGRA";
    case 4: // ImageColorMode::rgba:
        return "RGBA";
    }
    return "";
}
/*!
 * \brief cdの画像を変換しthisに送信
 *
 * cd.image[field]が更新されるかリクエストが更新されたときに変換を行う。
 *
 */
void MemberData::imageConvertThreadMain(const SharedString &member,
                                        const SharedString &field) {
    int last_image_flag = -1, last_req_flag = -1;
    logger->trace("imageConvertThreadMain started for {}, {}", member.decode(),
                  field.decode());
    while (true) {
        store->findAndDo(
            member,
            [&](auto cd) {
                while (!cd->closing.load() && !this->closing.load()) {
                    Message::ImageFrame img;
                    {
                        std::unique_lock lock(cd->image_m[field]);
                        cd->image_cv[field].wait_for(
                            lock, std::chrono::milliseconds(1));
                        if (cd->closing.load() || this->closing.load()) {
                            break;
                        }
                        if (cd->image_changed[field] == last_image_flag &&
                            this->image_req_changed[member][field] ==
                                last_req_flag) {
                            continue;
                        }
                        last_image_flag = cd->image_changed[field];
                        last_req_flag = this->image_req_changed[member][field];
                        logger->trace("converting image of {}, {}",
                                      member.decode(), field.decode());
                        img = cd->image[field];
                    }
                    if (img.data_->size() == 0) {
                        break;
                    }

                    Magick::Image m(img.width_, img.height_,
                                    magickColorMap(img.color_mode_),
                                    Magick::CharPixel, img.data_->data());
#ifdef WEBCFACE_MAGICK_VER7
                    // ImageMagick6と7で名前が異なる
                    m.type(Magick::TrueColorAlphaType);
#else
                    m.type(Magick::TrueColorMatteType);
#endif
                    if (img.color_mode_ == 0 /* gray */) {
                        // K -> RGB
                        m.negate(true);
                    }

                    auto last_frame = std::chrono::steady_clock::now();
                    // 変換処理
                    auto info = this->image_req_info[member][field];
                    // clang-tidyの偽陽性への対処のため構造化束縛しない
                    auto req_field =
                        findReqField(this->image_req, member, field);
                    auto &req_id = req_field.first;
                    auto &sub_field = req_field.second;
                    auto sync = webcface::Message::Sync{cd->member_id,
                                                        cd->last_sync_time};

                    int rows = static_cast<int>(img.height_);
                    int cols = static_cast<int>(img.width_);

                    if (info.rows || info.cols) {
                        if (info.rows) {
                            rows = *info.rows;
                        } else {
                            rows = static_cast<int>(
                                static_cast<double>(*info.cols) *
                                static_cast<double>(img.height_) /
                                static_cast<double>(img.width_));
                        }
                        if (info.cols) {
                            cols = *info.cols;
                        } else {
                            cols = static_cast<int>(
                                static_cast<double>(*info.rows) *
                                static_cast<double>(img.width_) /
                                static_cast<double>(img.height_));
                        }

                        if (rows <= 0 || cols <= 0) {
                            this->logger->error(
                                "Invalid image conversion request "
                                "(rows={}, cols={})",
                                rows, cols);
                            return;
                        }
                        m.resize(Magick::Geometry(cols, rows));
                    }

                    auto color_mode = info.color_mode.value_or(img.color_mode_);
                    auto encoded =
                        std::make_shared<std::vector<unsigned char>>();
                    switch (info.cmp_mode) {
                    case 0: { // ImageCompressMode::raw: {
                        std::size_t channels = 1;
                        std::string color_map = magickColorMap(color_mode);
                        switch (color_mode) {
                        case 0: // ImageColorMode::gray:
                            m.type(Magick::GrayscaleType);
                            color_map = "R";
                            channels = 1;
                            break;
                        case 1: // ImageColorMode::bgr:
                        case 3: // ImageColorMode::rgb:
                            channels = 3;
                            break;
                        case 2: // ImageColorMode::bgra:
                        case 4: // ImageColorMode::rgba:
                            channels = 4;
                            break;
                        }
                        encoded->resize(static_cast<std::size_t>(cols * rows) *
                                        channels);
                        m.write(0, 0, cols, rows, color_map, Magick::CharPixel,
                                encoded->data());
                        break;
                    }
                    case 1: { // ImageCompressMode::jpeg: {
                        if (info.quality < 0 || info.quality > 100) {
                            this->logger->error(
                                "Invalid image conversion request "
                                "(jpeg, quality={})",
                                info.quality);
                            return;
                        }
                        m.magick("JPEG");
                        m.quality(info.quality);
                        Magick::Blob b;
                        m.write(&b);
                        encoded->assign(
                            static_cast<const unsigned char *>(b.data()),
                            static_cast<const unsigned char *>(b.data()) +
                                b.length());
                        break;
                    }
                    case 2: { // ImageCompressMode::webp: {
                        if (info.quality < 1 || info.quality > 100) {
                            this->logger->error(
                                "Invalid image conversion request "
                                "(webp, quality={})",
                                info.quality);
                            return;
                        }
                        m.magick("WEBP");
                        m.quality(info.quality);
                        Magick::Blob b;
                        m.write(&b);
                        encoded->assign(
                            static_cast<const unsigned char *>(b.data()),
                            static_cast<const unsigned char *>(b.data()) +
                                b.length());
                        break;
                    }
                    case 3: { // ImageCompressMode::png: {
                        if (info.quality < 0 || info.quality > 100) {
                            this->logger->error(
                                "Invalid image conversion request "
                                "(png, compression={})",
                                info.quality);
                            return;
                        }
                        m.magick("PNG");
                        m.quality(info.quality);
                        Magick::Blob b;
                        m.write(&b);
                        encoded->assign(
                            static_cast<const unsigned char *>(b.data()),
                            static_cast<const unsigned char *>(b.data()) +
                                b.length());
                        break;
                    }
                    }
                    Message::ImageFrame img_send{
                        static_cast<size_t>(cols), static_cast<size_t>(rows),
                        encoded, color_mode, info.cmp_mode};
                    logger->trace("finished converting image of {}, {}",
                                  member.decode(), field.decode());
                    if (!cd->closing.load() && !this->closing.load()) {
                        std::lock_guard lock(store->server->server_mtx);
                        this->pack(sync);
                        this->pack(Message::Res<webcface::Message::Image>{
                            req_id, sub_field, img_send});
                        logger->trace("send image_res req_id={} + '{}'", req_id,
                                      sub_field.decode());
                        this->send();
                    }
                    if (info.frame_rate && *info.frame_rate > 0) {
                        std::chrono::milliseconds delay{
                            static_cast<int>(1000 / *info.frame_rate)};
                        while (std::chrono::duration_cast<
                                   std::chrono::milliseconds>(
                                   std::chrono::steady_clock::now() -
                                   last_frame) < delay &&
                               !cd->closing.load() && !this->closing.load()) {
                            std::this_thread::sleep_for(
                                std::chrono::milliseconds(1));
                        }
                        // last_frame = std::chrono::steady_clock::now();
                    }
                }
            },
            [] { std::chrono::milliseconds(1); });
        if (this->closing.load()) {
            break;
        }
    }
}
} // namespace Server
WEBCFACE_NS_END

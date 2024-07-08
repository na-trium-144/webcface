#include "webcface/server/member_data.h"
#include "webcface/server/store.h"
#include <webcface/server/server.h>
#include "webcface/message/message.h"
#include <webcface/common/def.h>
#include <algorithm>

WEBCFACE_NS_BEGIN
namespace server {

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
            auto v = std::any_cast<webcface::message::SyncInit>(obj);
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
            this->pack(webcface::message::SvrVersion{
                {}, WEBCFACE_SERVER_NAME, WEBCFACE_VERSION});
            // 逆に新しいMemberに他の全Memberのentryを通知
            store->forEachWithName([&](auto cd) {
                if (cd->member_id != this->member_id) {
                    this->pack(cd->init_data);
                    logger->trace("send sync_init {} ({})", cd->name_s,
                                  cd->member_id);

                    for (const auto &f : cd->value) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Value>{
                                {}, cd->member_id, f.first});
                            logger->trace("send value_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->text) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Text>{
                                {}, cd->member_id, f.first});
                            logger->trace("send text_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->robot_model) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::RobotModel>{
                                {}, cd->member_id, f.first});
                            logger->trace(
                                "send robot_model_entry {} of member {}",
                                f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas3d) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Canvas3D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas3d_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas2d) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Canvas2D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas2d_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->view) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::View>{
                                {}, cd->member_id, f.first});
                            logger->trace("send view_entry {} of member {}",
                                          f.first.decode(), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->image) {
                        if (!f.first.u8String().starts_with(field_separator)) {
                            this->pack(webcface::message::Entry<
                                       webcface::message::Image>{
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
            auto v = std::any_cast<webcface::message::Sync>(obj);
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
            auto v = std::any_cast<webcface::message::Call>(obj);
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
            auto v = std::any_cast<webcface::message::CallResponse>(obj);
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
            auto v = std::any_cast<webcface::message::CallResult>(obj);
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
            auto v = std::any_cast<webcface::message::Value>(obj);
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
            auto v = std::any_cast<webcface::message::Text>(obj);
            logger->debug("text {} = {}", v.field.decode(),
                          static_cast<std::string>(*v.data));
            if (!this->text.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
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
            auto v = std::any_cast<webcface::message::RobotModel>(obj);
            logger->debug("robot model {}", v.field.decode());
            if (!this->robot_model.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
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
            auto v = std::any_cast<webcface::message::View>(obj);
            logger->debug("view {} diff={}, length={}", v.field.decode(),
                          v.data_diff->size(), v.length);
            if (!this->view.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::message::Entry<webcface::message::View>{
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
                auto req_field =
                    findReqField(cd->view_req, this->name, v.field);
                auto &req_id = req_field.first;
                auto &sub_field = req_field.second;
                if (req_id > 0) {
                    cd->pack(webcface::message::Res<webcface::message::View>(
                        req_id, sub_field, v.data_diff, v.length));
                    cd->logger->trace("send view_res req_id={} + '{}'", req_id,
                                      sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::canvas3d: {
            auto v = std::any_cast<webcface::message::Canvas3D>(obj);
            logger->debug("canvas3d {} diff={}, length={}", v.field.decode(),
                          v.data_diff->size(), v.length);
            if (!this->canvas3d.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas3D>{
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
                auto req_field =
                    findReqField(cd->canvas3d_req, this->name, v.field);
                auto &req_id = req_field.first;
                auto &sub_field = req_field.second;
                if (req_id > 0) {
                    cd->pack(
                        webcface::message::Res<webcface::message::Canvas3D>(
                            req_id, sub_field, v.data_diff, v.length));
                    cd->logger->trace("send canvas3d_res req_id={} + '{}'",
                                      req_id, sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::canvas2d: {
            auto v = std::any_cast<webcface::message::Canvas2D>(obj);
            logger->debug("canvas2d {} diff={}, length={}", v.field.decode(),
                          v.data_diff->size(), v.length);
            if (!this->canvas2d.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::message::Entry<
                                 webcface::message::Canvas2D>{
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
                auto req_field =
                    findReqField(cd->canvas2d_req, this->name, v.field);
                auto &req_id = req_field.first;
                auto &sub_field = req_field.second;
                if (req_id > 0) {
                    cd->pack(
                        webcface::message::Res<webcface::message::Canvas2D>(
                            req_id, sub_field, v.width, v.height, v.data_diff,
                            v.length));
                    cd->logger->trace("send canvas2d_res req_id={} + '{}'",
                                      req_id, sub_field.decode());
                }
            });
            break;
        }
        case MessageKind::image: {
            auto v = std::any_cast<webcface::message::Image>(obj);
            logger->debug("image {} ({} x {})", v.field.decode(), v.width_,
                          v.height_);
            if (!this->image.count(v.field) &&
                !v.field.u8String().starts_with(field_separator)) {
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
        case MessageKind::log: {
            auto v = std::any_cast<webcface::message::Log>(obj);
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
            auto v = std::any_cast<webcface::message::FuncInfo>(obj);
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
            this->func[v.field] = std::make_shared<message::FuncInfo>(v);
            break;
        }
        case MessageKind::req + MessageKind::value: {
            auto s =
                std::any_cast<webcface::message::Req<webcface::message::Value>>(
                    obj);
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
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
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
            auto s =
                std::any_cast<webcface::message::Req<webcface::message::Text>>(
                    obj);
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
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
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
            auto s = std::any_cast<
                webcface::message::Req<webcface::message::RobotModel>>(obj);
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
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        SharedString sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = SharedString(it.first.u8String().substr(
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
            auto s =
                std::any_cast<webcface::message::Req<webcface::message::View>>(
                    obj);
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
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string, webcface::message::ViewComponent>>();
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
                            webcface::message::Res<webcface::message::View>{
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
                webcface::message::Req<webcface::message::Canvas3D>>(obj);
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
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            webcface::message::Canvas3DComponent>>();
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
                            webcface::message::Res<webcface::message::Canvas3D>{
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
                webcface::message::Req<webcface::message::Canvas2D>>(obj);
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
                        it.first.u8String().starts_with(s.field.u8String() +
                                                        field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            webcface::message::Canvas2DComponent>>();
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
                            webcface::message::Res<webcface::message::Canvas2D>{
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
                std::any_cast<webcface::message::Req<webcface::message::Image>>(
                    obj);
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
        case MessageKind::log_req: {
            auto s = std::any_cast<webcface::message::LogReq>(obj);
            logger->debug("request log from {}", s.member.decode());
            log_req.insert(s.member);
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                this->pack(webcface::message::Log{cd->member_id, cd->log});
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

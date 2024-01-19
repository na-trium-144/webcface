#include "s_client_data.h"
#include "store.h"
#include "websock.h"
#include "../message/message.h"
#include <webcface/common/def.h>
#include <algorithm>
#include <iterator>
#include <utf8.h>

#if WEBCFACE_USE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

namespace WEBCFACE_NS::Server {
void ClientData::onClose() {
    con = nullptr;
    logger->info("connection closed");
    closing.store(true);
    for (const auto &pm : pending_calls) {
        store.findAndDo(pm.first, [&](auto cd) {
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
                        "member(\"" + this->name + "\") Disconnected"}));
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
void ClientData::send() {
    if (connected() && send_len > 0) {
        send(Message::packDone(send_buffer, send_len));
    }
    send_buffer.str("");
    send_len = 0;
}
void ClientData::send(const std::string &msg) {
    if (connected()) {
        serverSend(con, msg);
    }
}
void ClientData::onConnect() { logger->debug("websocket connected"); }

bool ClientData::hasReq(const std::string &member) {
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
                       [](const auto &it) { return it.second > 0; });
}

std::pair<unsigned int, std::string> findReqField(
    std::unordered_map<std::string,
                       std::unordered_map<std::string, unsigned int>> &req,
    const std::string &member, const std::string &field) {
    for (const auto &req_it : req[member]) {
        if (req_it.first == field) {
            return std::make_pair(req_it.second, "");
        } else if (req_it.first.starts_with(field + ".")) {
            return std::make_pair(req_it.second,
                                  req_it.first.substr(field.size() + 1));
        }
    }
    return std::make_pair<unsigned int, std::string>(0, "");
}

void ClientData::sendPing() {
    last_send_ping = std::chrono::system_clock::now();
    last_ping_duration = std::nullopt;
    send(Message::packSingle(Message::Ping{}));
}
void ClientData::onRecv(const std::string &message) {
    namespace MessageKind = WEBCFACE_NS::Message::MessageKind;
    auto messages = WEBCFACE_NS::Message::unpack(message, this->logger);
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
            if (ping_status != nullptr) {
                this->pack(Message::PingStatus{{}, ping_status});
                logger->trace("send ping_status");
            }
            break;
        }
        case MessageKind::sync_init: {
            auto v = std::any_cast<WEBCFACE_NS::Message::SyncInit>(obj);
            this->name = v.member_name;
            auto member_id_before = this->member_id;
            auto prev_cli_it = std::find_if(
                store.clients_by_id.begin(), store.clients_by_id.end(),
                [&](const auto &it) { return it.second->name == this->name; });
            if (prev_cli_it != store.clients_by_id.end()) {
                this->member_id = v.member_id = prev_cli_it->first;
            } else {
                // コンストラクタですでに一意のidが振られているはず
                v.member_id = this->member_id;
            }
            v.addr = this->remote_addr;
            this->init_data = v;
            this->sync_init = true;
            store.clients_by_id.erase(this->member_id);
            store.clients_by_id.emplace(this->member_id, store.getClient(con));
            if (this->name == "") {
                logger->debug("sync_init (no name)");
            } else {
                this->logger = std::make_shared<spdlog::logger>(
                    std::to_string(this->member_id) + "_" + this->name,
                    this->sink);
                this->logger->set_level(this->logger_level);
                this->logger->debug(
                    "sync_init name={}, member_id={} (before {})", this->name,
                    this->member_id, member_id_before);
                this->logger->info("successfully connected and initialized.");
                // 全クライアントに新しいMemberを通知
                store.forEach([&](auto cd) {
                    if (cd->member_id != this->member_id) {
                        cd->pack(v);
                        cd->logger->trace("send sync_init {} ({})", this->name,
                                          this->member_id);
                    }
                });
            }
            this->pack(WEBCFACE_NS::Message::SvrVersion{
                {}, WEBCFACE_SERVER_NAME, WEBCFACE_VERSION});
            // 逆に新しいMemberに他の全Memberのentryを通知
            store.forEachWithName([&](auto cd) {
                if (cd->member_id != this->member_id) {
                    this->pack(cd->init_data);
                    logger->trace("send sync_init {} ({})", cd->name,
                                  cd->member_id);

                    for (const auto &f : cd->value) {
                        this->pack(WEBCFACE_NS::Message::Entry<
                                   WEBCFACE_NS::Message::Value>{
                            {}, cd->member_id, f.first});
                        logger->trace("send value_entry {} of member {}",
                                      f.first, cd->member_id);
                    }
                    for (const auto &f : cd->text) {
                        this->pack(WEBCFACE_NS::Message::Entry<
                                   WEBCFACE_NS::Message::Text>{
                            {}, cd->member_id, f.first});
                        logger->trace("send text_entry {} of member {}",
                                      f.first, cd->member_id);
                    }
                    for (const auto &f : cd->robot_model) {
                        this->pack(WEBCFACE_NS::Message::Entry<
                                   WEBCFACE_NS::Message::RobotModel>{
                            {}, cd->member_id, f.first});
                        logger->trace("send robot_model_entry {} of member {}",
                                      f.first, cd->member_id);
                    }
                    for (const auto &f : cd->canvas3d) {
                        this->pack(WEBCFACE_NS::Message::Entry<
                                   WEBCFACE_NS::Message::Canvas3D>{
                            {}, cd->member_id, f.first});
                        logger->trace("send canvas3d_entry {} of member {}",
                                      f.first, cd->member_id);
                    }
                    for (const auto &f : cd->view) {
                        this->pack(WEBCFACE_NS::Message::Entry<
                                   WEBCFACE_NS::Message::View>{
                            {}, cd->member_id, f.first});
                        logger->trace("send view_entry {} of member {}",
                                      f.first, cd->member_id);
                    }
                    for (const auto &f : cd->image) {
                        this->pack(WEBCFACE_NS::Message::Entry<
                                   WEBCFACE_NS::Message::Image>{
                            {}, cd->member_id, f.first});
                        logger->trace("send image_entry {} of member {}",
                                      f.first, cd->member_id);
                    }
                    for (const auto &f : cd->func) {
                        this->pack(*f.second);
                        logger->trace("send func_info {} of member {}",
                                      f.second->field, cd->member_id);
                    }
                }
            });
            break;
        }
        case MessageKind::sync: {
            auto v = std::any_cast<WEBCFACE_NS::Message::Sync>(obj);
            v.member_id = this->member_id;
            logger->debug("sync");
            // 1つ以上リクエストしているクライアントにはsyncの情報を流す
            store.forEach([&](auto cd) {
                if (cd->hasReq(this->name)) {
                    cd->pack(v);
                    cd->logger->trace("send sync {}", this->member_id);
                }
            });
            break;
        }
        case MessageKind::call: {
            auto v = std::any_cast<WEBCFACE_NS::Message::Call>(obj);
            v.caller_member_id = this->member_id;
            for (auto &a : v.args) {
                a = utf8::replace_invalid(static_cast<std::string>(a));
            }
            logger->debug(
                "call caller_id={}, target_id={}, field={}, with {} args",
                v.caller_id, v.target_member_id, v.field, v.args.size());
            // そのままターゲットのクライアントに送る
            store.findConnectedAndDo(
                v.target_member_id,
                [&](auto cd) {
                    cd->pack(v);
                    cd->pending_calls[this->member_id][v.caller_id] = 2;
                    cd->logger->trace("send call caller_id={}, target_id={}, "
                                      "field={}, with {} args",
                                      v.caller_id, v.target_member_id, v.field,
                                      v.args.size());
                },
                [&]() {
                    // 関数存在しないor切断されているときの処理
                    this->pack(WEBCFACE_NS::Message::CallResponse{
                        {}, v.caller_id, v.caller_member_id, false});
                    logger->debug("call target not found");
                });
            break;
        }
        case MessageKind::call_response: {
            auto v = std::any_cast<WEBCFACE_NS::Message::CallResponse>(obj);
            logger->debug("call_response to (member_id {}, caller_id {}), {}",
                          v.caller_member_id, v.caller_id, v.started);
            this->pending_calls[v.caller_member_id][v.caller_id] = 1;
            // そのままcallerに送る
            store.findAndDo(v.caller_member_id, [&](auto cd) {
                cd->pack(v);
                cd->logger->trace(
                    "send call_response to (member_id {}, caller_id {}), {}",
                    v.caller_member_id, v.caller_id, v.started);
            });
            break;
        }
        case MessageKind::call_result: {
            auto v = std::any_cast<WEBCFACE_NS::Message::CallResult>(obj);
            v.result =
                utf8::replace_invalid(static_cast<std::string>(v.result));
            logger->debug("call_result to (member_id {}, caller_id {}), {}",
                          v.caller_member_id, v.caller_id,
                          static_cast<std::string>(v.result));
            this->pending_calls[v.caller_member_id][v.caller_id] = 0;
            // そのままcallerに送る
            store.findAndDo(v.caller_member_id, [&](auto cd) {
                cd->pack(v);
                cd->logger->trace(
                    "send call_result to (member_id {}, caller_id {}), {}",
                    v.caller_member_id, v.caller_id,
                    static_cast<std::string>(v.result));
            });
            break;
        }
        case MessageKind::value: {
            auto v = std::any_cast<WEBCFACE_NS::Message::Value>(obj);
            if (v.data->size() == 1) {
                logger->debug("value {} = {}", v.field, (*v.data)[0]);
            } else {
                logger->debug("value {} = (array length = {})", v.field,
                              v.data->size());
            }
            if (!this->value.count(v.field)) {
                store.forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(WEBCFACE_NS::Message::Entry<
                                 WEBCFACE_NS::Message::Value>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send value_entry {} of member {}",
                                          v.field, this->member_id);
                    }
                });
            }
            this->value[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->value_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(
                        WEBCFACE_NS::Message::Res<WEBCFACE_NS::Message::Value>(
                            req_id, sub_field, v.data));
                    cd->logger->trace("send value_res req_id={} + '{}'", req_id,
                                      sub_field);
                }
            });
            break;
        }
        case MessageKind::text: {
            auto v = std::any_cast<WEBCFACE_NS::Message::Text>(obj);
            *v.data = utf8::replace_invalid(*v.data);
            logger->debug("text {} = {}", v.field, *v.data);
            if (!this->text.count(v.field)) {
                store.forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(WEBCFACE_NS::Message::Entry<
                                 WEBCFACE_NS::Message::Text>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send text_entry {} of member {}",
                                          v.field, this->member_id);
                    }
                });
            }
            this->text[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->text_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(
                        WEBCFACE_NS::Message::Res<WEBCFACE_NS::Message::Text>(
                            req_id, sub_field, v.data));
                    cd->logger->trace("send text_res {}, req_id={} + '{}'",
                                      *v.data, req_id, sub_field);
                }
            });
            break;
        }
        case MessageKind::robot_model: {
            auto v = std::any_cast<WEBCFACE_NS::Message::RobotModel>(obj);
            logger->debug("robot model {}", v.field);
            if (!this->robot_model.count(v.field)) {
                store.forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(WEBCFACE_NS::Message::Entry<
                                 WEBCFACE_NS::Message::RobotModel>{
                            {}, this->member_id, v.field});
                        cd->logger->trace(
                            "send robot_model_entry {} of member {}", v.field,
                            this->member_id);
                    }
                });
            }
            this->robot_model[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->robot_model_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(WEBCFACE_NS::Message::Res<
                             WEBCFACE_NS::Message::RobotModel>(
                        req_id, sub_field, v.data));
                    cd->logger->trace("send robot_model_res, req_id={} + '{}'",
                                      req_id, sub_field);
                }
            });
            break;
        }
        case MessageKind::view: {
            auto v = std::any_cast<WEBCFACE_NS::Message::View>(obj);
            logger->debug("view {} diff={}, length={}", v.field,
                          v.data_diff->size(), v.length);
            if (!this->view.count(v.field)) {
                store.forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(WEBCFACE_NS::Message::Entry<
                                 WEBCFACE_NS::Message::View>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send view_entry {} of member {}",
                                          v.field, this->member_id);
                    }
                });
            }
            this->view[v.field].resize(v.length);
            for (auto &d : *v.data_diff) {
                d.second.text = utf8::replace_invalid(d.second.text);
                this->view[v.field][std::stoi(d.first)] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->view_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(
                        WEBCFACE_NS::Message::Res<WEBCFACE_NS::Message::View>(
                            req_id, sub_field, v.data_diff, v.length));
                    cd->logger->trace("send view_res req_id={} + '{}'", req_id,
                                      sub_field);
                }
            });
            break;
        }
        case MessageKind::canvas3d: {
            auto v = std::any_cast<WEBCFACE_NS::Message::Canvas3D>(obj);
            logger->debug("canvas3d {} diff={}, length={}", v.field,
                          v.data_diff->size(), v.length);
            if (!this->canvas3d.count(v.field)) {
                store.forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(WEBCFACE_NS::Message::Entry<
                                 WEBCFACE_NS::Message::Canvas3D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas3d_entry {} of member {}",
                                          v.field, this->member_id);
                    }
                });
            }
            this->canvas3d[v.field].resize(v.length);
            for (const auto &d : *v.data_diff) {
                this->canvas3d[v.field][std::stoi(d.first)] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto cd) {
                auto [req_id, sub_field] =
                    findReqField(cd->canvas3d_req, this->name, v.field);
                if (req_id > 0) {
                    cd->pack(WEBCFACE_NS::Message::Res<
                             WEBCFACE_NS::Message::Canvas3D>(
                        req_id, sub_field, v.data_diff, v.length));
                    cd->logger->trace("send canvas3d_res req_id={} + '{}'",
                                      req_id, sub_field);
                }
            });
            break;
        }
        case MessageKind::image: {
            auto v = std::any_cast<WEBCFACE_NS::Message::Image>(obj);
            logger->debug("image {} ({} x {} x {})", v.field, v.rows(),
                          v.cols(), v.channels());
            if (!this->image.count(v.field)) {
                store.forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(WEBCFACE_NS::Message::Entry<
                                 WEBCFACE_NS::Message::Image>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send image_entry {} of member {}",
                                          v.field, this->member_id);
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
            auto v = std::any_cast<WEBCFACE_NS::Message::Log>(obj);
            v.member_id = this->member_id;
            logger->debug("log {} lines", v.log->size());
            if (store.keep_log >= 0 &&
                this->log->size() < static_cast<unsigned int>(store.keep_log) &&
                this->log->size() + v.log->size() >=
                    static_cast<unsigned int>(store.keep_log)) {
                logger->info("number of log lines reached {}, so the oldest "
                             "log will be romoved.",
                             store.keep_log);
            }
            for (auto &ll : *v.log) {
                ll.message = utf8::replace_invalid(ll.message);
                this->log->push_back(ll);
            }
            while (store.keep_log >= 0 &&
                   this->log->size() >
                       static_cast<unsigned int>(store.keep_log)) {
                this->log->pop_front();
            }
            // このlogをsubscribeしてるところに送り返す
            store.forEach([&](auto cd) {
                if (cd->log_req.count(this->name)) {
                    cd->pack(v);
                    cd->logger->trace("send log {} lines", v.log->size());
                }
            });
            break;
        }
        case MessageKind::func_info: {
            auto v = std::any_cast<WEBCFACE_NS::Message::FuncInfo>(obj);
            v.member_id = this->member_id;
            for (auto &a : *v.args) {
                std::vector<Common::ValAdaptor> replaced_opt;
                for (auto &o : a.option()) {
                    replaced_opt.push_back(
                        utf8::replace_invalid(static_cast<std::string>(o)));
                }
                a = Common::Arg(
                    utf8::replace_invalid(a.name()), a.type(),
                    a.init() ? std::make_optional<Common::ValAdaptor>(
                                   utf8::replace_invalid(
                                       static_cast<std::string>(*a.init())))
                             : std::nullopt,
                    a.min(), a.max(), replaced_opt);
            }
            logger->debug("func_info {}", v.field);
            if (!this->func.count(v.field)) {
                store.forEach([&](auto cd) {
                    if (cd->member_id != this->member_id) {
                        cd->pack(v);
                        cd->logger->trace("send func_info {} of member {}",
                                          v.field, this->member_id);
                    }
                });
            }
            this->func[v.field] = std::make_shared<Message::FuncInfo>(v);
            break;
        }
        case MessageKind::req + MessageKind::value: {
            auto s = std::any_cast<
                WEBCFACE_NS::Message::Req<WEBCFACE_NS::Message::Value>>(obj);
            logger->debug("request value ({}): {} from {}", s.req_id, s.field,
                          s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WEBCFACE_NS::Message::Sync{cd->member_id,
                                                          cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->value) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(WEBCFACE_NS::Message::Res<
                                   WEBCFACE_NS::Message::Value>{
                            s.req_id, sub_field, it.second});
                        logger->trace("send value_res req_id={} + '{}'",
                                      s.req_id, sub_field);
                    }
                }
            });
            value_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::text: {
            auto s = std::any_cast<
                WEBCFACE_NS::Message::Req<WEBCFACE_NS::Message::Text>>(obj);
            logger->debug("request text ({}): {} from {}", s.req_id, s.field,
                          s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WEBCFACE_NS::Message::Sync{cd->member_id,
                                                          cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->text) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(WEBCFACE_NS::Message::Res<
                                   WEBCFACE_NS::Message::Text>{
                            s.req_id, sub_field, it.second});
                        logger->trace("send text_res {}, req_id={} + '{}'",
                                      *it.second, s.req_id, sub_field);
                    }
                }
            });
            text_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::robot_model: {
            auto s = std::any_cast<
                WEBCFACE_NS::Message::Req<WEBCFACE_NS::Message::RobotModel>>(
                obj);
            logger->debug("request robot_model ({}): {} from {}", s.req_id,
                          s.field, s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WEBCFACE_NS::Message::Sync{cd->member_id,
                                                          cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->robot_model) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(WEBCFACE_NS::Message::Res<
                                   WEBCFACE_NS::Message::RobotModel>{
                            s.req_id, sub_field, it.second});
                        logger->trace("send robot_model_res, req_id={} + '{}'",
                                      s.req_id, sub_field);
                    }
                }
            });
            robot_model_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::view: {
            auto s = std::any_cast<
                WEBCFACE_NS::Message::Req<WEBCFACE_NS::Message::View>>(obj);
            logger->debug("request view ({}): {} from {}", s.req_id, s.field,
                          s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WEBCFACE_NS::Message::Sync{cd->member_id,
                                                          cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->view) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            WEBCFACE_NS::Message::View::ViewComponent>>();
                        for (std::size_t i = 0; i < it.second.size(); i++) {
                            diff->emplace(std::to_string(i), it.second[i]);
                        }
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(WEBCFACE_NS::Message::Res<
                                   WEBCFACE_NS::Message::View>{
                            s.req_id, sub_field, diff, it.second.size()});
                        logger->trace("send view_res req_id={} + '{}'",
                                      s.req_id, sub_field);
                    }
                }
            });
            view_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::canvas3d: {
            auto s = std::any_cast<
                WEBCFACE_NS::Message::Req<WEBCFACE_NS::Message::Canvas3D>>(obj);
            logger->debug("request canvas3d ({}): {} from {}", s.req_id,
                          s.field, s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WEBCFACE_NS::Message::Sync{cd->member_id,
                                                          cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas3d) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string, WEBCFACE_NS::Message::Canvas3D::
                                             Canvas3DComponent>>();
                        for (std::size_t i = 0; i < it.second.size(); i++) {
                            diff->emplace(std::to_string(i), it.second[i]);
                        }
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(WEBCFACE_NS::Message::Res<
                                   WEBCFACE_NS::Message::Canvas3D>{
                            s.req_id, sub_field, diff, it.second.size()});
                        logger->trace("send canvas3d_res req_id={} + '{}'",
                                      s.req_id, sub_field);
                    }
                }
            });
            canvas3d_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::image: {
            auto s = std::any_cast<
                WEBCFACE_NS::Message::Req<WEBCFACE_NS::Message::Image>>(obj);
            logger->debug("request image ({}): {} from {}, {} x {}, color={}, "
                          "mode={}, q={}, fps={}",
                          s.req_id, s.field, s.member, s.rows.value_or(-1),
                          s.cols.value_or(-1),
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
            auto s = std::any_cast<WEBCFACE_NS::Message::LogReq>(obj);
            logger->debug("request log from {}", s.member);
            log_req.insert(s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto cd) {
                this->pack(WEBCFACE_NS::Message::Log{cd->member_id, cd->log});
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
        case MessageKind::entry + MessageKind::image:
        case MessageKind::res + MessageKind::image:
        case MessageKind::svr_version:
        case MessageKind::ping_status:
            logger->warn("Invalid Message Kind {}", kind);
            break;
        default:
            logger->warn("Unknown Message Kind {}", kind);
            break;
        }
    }
    store.clientSendAll();
}

#if WEBCFACE_USE_OPENCV
static int colorConvert(Common::ImageColorMode src_mode,
                        Common::ImageColorMode dst_mode) {
    switch (src_mode) {
    case Common::ImageColorMode::gray:
        switch (dst_mode) {
        case Common::ImageColorMode::bgr:
            return cv::COLOR_GRAY2BGR;
        case Common::ImageColorMode::bgra:
            return cv::COLOR_GRAY2BGRA;
        case Common::ImageColorMode::rgb:
            return cv::COLOR_GRAY2RGB;
        case Common::ImageColorMode::rgba:
            return cv::COLOR_GRAY2RGBA;
        case Common::ImageColorMode::gray:
            break;
        }
        break;
    case Common::ImageColorMode::bgr:
        switch (dst_mode) {
        case Common::ImageColorMode::gray:
            return cv::COLOR_BGR2GRAY;
        case Common::ImageColorMode::bgra:
            return cv::COLOR_BGR2BGRA;
        case Common::ImageColorMode::rgb:
            return cv::COLOR_BGR2RGB;
        case Common::ImageColorMode::rgba:
            return cv::COLOR_BGR2RGBA;
        case Common::ImageColorMode::bgr:
            break;
        }
        break;
    case Common::ImageColorMode::bgra:
        switch (dst_mode) {
        case Common::ImageColorMode::gray:
            return cv::COLOR_BGRA2GRAY;
        case Common::ImageColorMode::bgr:
            return cv::COLOR_BGRA2BGR;
        case Common::ImageColorMode::rgb:
            return cv::COLOR_BGRA2RGB;
        case Common::ImageColorMode::rgba:
            return cv::COLOR_BGRA2RGBA;
        case Common::ImageColorMode::bgra:
            break;
        }
        break;
    case Common::ImageColorMode::rgb:
        switch (dst_mode) {
        case Common::ImageColorMode::gray:
            return cv::COLOR_RGB2GRAY;
        case Common::ImageColorMode::bgr:
            return cv::COLOR_RGB2BGR;
        case Common::ImageColorMode::bgra:
            return cv::COLOR_RGB2BGRA;
        case Common::ImageColorMode::rgba:
            return cv::COLOR_RGB2RGBA;
        case Common::ImageColorMode::rgb:
            break;
        }
        break;
    case Common::ImageColorMode::rgba:
        switch (dst_mode) {
        case Common::ImageColorMode::gray:
            return cv::COLOR_RGBA2GRAY;
        case Common::ImageColorMode::bgr:
            return cv::COLOR_RGBA2BGR;
        case Common::ImageColorMode::bgra:
            return cv::COLOR_RGBA2BGRA;
        case Common::ImageColorMode::rgb:
            return cv::COLOR_RGBA2RGB;
        case Common::ImageColorMode::rgba:
            break;
        }
        break;
    }
    return -1;
}
#endif

void ClientData::imageConvertThreadMain(const std::string &member,
                                        const std::string &field) {
    // cdの画像を変換しthisに送信
    // cd.image[field]が更新されるかリクエストが更新されたときに変換を行う。
    int last_image_flag = -1, last_req_flag = -1;
    logger->trace("imageConvertThreadMain started for {}, {}", member, field);
    while (true) {
        store.findAndDo(member, [&](auto cd) {
            while (!cd->closing.load() && !this->closing.load()) {
                Common::ImageFrame img;
                {
                    std::unique_lock lock(cd->image_m[field]);
                    cd->image_cv[field].wait_for(lock,
                                                 std::chrono::milliseconds(1));
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
                    logger->trace("converting image of {}, {}", member, field);
                    img = cd->image[field];
                }
                if (img.empty()) {
                    break;
                }
#if WEBCFACE_USE_OPENCV
                cv::Mat m = img.mat();
#endif
                auto last_frame = std::chrono::steady_clock::now();
                // 変換処理
                auto info = this->image_req_info[member][field];
                auto [req_id, sub_field] =
                    findReqField(this->image_req, member, field);
                auto sync = WEBCFACE_NS::Message::Sync{cd->member_id,
                                                       cd->last_sync_time};

                int rows = static_cast<int>(img.rows()),
                    cols = static_cast<int>(img.cols());

                if (info.rows || info.cols) {
#if WEBCFACE_USE_OPENCV
                    if (info.rows) {
                        rows = *info.rows;
                    } else {
                        rows = static_cast<int>(
                            static_cast<double>(*info.cols) * m.rows / m.cols);
                    }
                    if (info.cols) {
                        cols = *info.cols;
                    } else {
                        cols = static_cast<int>(
                            static_cast<double>(*info.rows) * m.cols / m.rows);
                    }

                    if (rows <= 0 || cols <= 0) {
                        this->logger->error("Invalid image conversion request "
                                            "(rows={}, cols={})",
                                            rows, cols);
                        return;
                    }
                    cv::resize(m, m, cv::Size(cols, rows));
#else
                    this->logger->warn("Cannot convert image since OpenCV is disabled.");
                    return;
#endif
                }
                if (info.color_mode && *info.color_mode != img.color_mode()) {
#if WEBCFACE_USE_OPENCV
                    cv::cvtColor(
                        m, m, colorConvert(img.color_mode(), *info.color_mode));
#else
                    this->logger->warn("Cannot convert image since OpenCV is disabled.");
                    return;
#endif
                }
                auto encoded = std::make_shared<std::vector<unsigned char>>();
                switch (info.cmp_mode) {
#if WEBCFACE_USE_OPENCV
                case Common::ImageCompressMode::raw:
                    encoded->assign(reinterpret_cast<unsigned char *>(m.data),
                                    reinterpret_cast<unsigned char *>(m.data) +
                                        m.total() * m.channels());
                    break;
                case Common::ImageCompressMode::jpeg:
                    if (info.quality < 0 || info.quality > 100) {
                        this->logger->error("Invalid image conversion request "
                                            "(jpeg, quality={})",
                                            info.quality);
                        return;
                    }
                    cv::imencode(".jpg", m, *encoded,
                                 {cv::IMWRITE_JPEG_QUALITY, info.quality});
                    break;
                case Common::ImageCompressMode::webp:
                    if (info.quality < 1 || info.quality > 100) {
                        this->logger->error("Invalid image conversion request "
                                            "(webp, quality={})",
                                            info.quality);
                        return;
                    }
                    cv::imencode(".webp", m, *encoded,
                                 {cv::IMWRITE_WEBP_QUALITY, info.quality});
                    break;
                case Common::ImageCompressMode::png:
                    if (info.quality < 0 || info.quality > 9) {
                        this->logger->error("Invalid image conversion request "
                                            "(png, compression={})",
                                            info.quality);
                        return;
                    }
                    cv::imencode(".png", m, *encoded,
                                 {cv::IMWRITE_PNG_COMPRESSION, info.quality});
                    break;
#else
                case Common::ImageCompressMode::raw:
                    encoded = img.dataPtr();
                    break;
                default:
                    this->logger->warn("Cannot convert image since OpenCV is disabled.");
                    return;
#endif
                }
                Common::ImageBase img_send{
                    rows, cols, encoded,
                    info.color_mode.value_or(img.color_mode()), info.cmp_mode};

                while (!cd->closing.load() && !this->closing.load()) {
                    if (server_mtx.try_lock()) {
                        this->pack(sync);
                        this->pack(WEBCFACE_NS::Message::Res<
                                   WEBCFACE_NS::Message::Image>{
                            req_id, sub_field, img_send});
                        logger->trace("send image_res req_id={} + '{}'", req_id,
                                      sub_field);
                        this->send();
                        server_mtx.unlock();
                        break;
                    } else {
                        std::this_thread::yield();
                    }
                }
                if (info.frame_rate && *info.frame_rate > 0) {
                    std::chrono::milliseconds delay{
                        static_cast<int>(1000 / *info.frame_rate)};
                    while (
                        std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - last_frame) <
                            delay &&
                        !cd->closing.load() && !this->closing.load()) {
                        std::this_thread::sleep_for(
                            std::chrono::milliseconds(1));
                    }
                    // last_frame = std::chrono::steady_clock::now();
                }
            }
        });
        if (this->closing.load()) {
            break;
        }
        std::this_thread::yield();
    }
}
} // namespace WEBCFACE_NS::Server

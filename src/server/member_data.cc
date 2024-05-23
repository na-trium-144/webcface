#include "member_data.h"
#include "store.h"
#include <webcface/server.h>
#include "../message/message.h"
#include "webcface/field.h"
#include <webcface/common/def.h>
#include <algorithm>
#include <iterator>

#if WEBCFACE_USE_OPENCV
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#endif

WEBCFACE_NS_BEGIN
namespace Server {
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
                        ValAdaptor{u8"member(\"" + this->name +
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

bool MemberData::hasReq(const std::u8string &member) {
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

std::pair<unsigned int, std::u8string> findReqField(
    std::unordered_map<std::u8string,
                       std::unordered_map<std::u8string, unsigned int>> &req,
    const std::u8string &member, const std::u8string &field) {
    for (const auto &req_it : req[member]) {
        if (req_it.first == field) {
            return std::make_pair(req_it.second, u8"");
        } else if (req_it.first.starts_with(field + field_separator)) {
            return std::make_pair(req_it.second,
                                  req_it.first.substr(field.size() + 1));
        }
    }
    return std::make_pair<unsigned int, std::u8string>(0, u8"");
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
            this->name_s = Encoding::decode(this->name = v.member_name);
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
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Value>{
                                {}, cd->member_id, f.first});
                            logger->trace("send value_entry {} of member {}",
                                          Encoding::decode(f.first),
                                          cd->member_id);
                        }
                    }
                    for (const auto &f : cd->text) {
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Text>{
                                {}, cd->member_id, f.first});
                            logger->trace("send text_entry {} of member {}",
                                          Encoding::decode(f.first),
                                          cd->member_id);
                        }
                    }
                    for (const auto &f : cd->robot_model) {
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::RobotModel>{
                                {}, cd->member_id, f.first});
                            logger->trace(
                                "send robot_model_entry {} of member {}",
                                Encoding::decode(f.first), cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas3d) {
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Canvas3D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas3d_entry {} of member {}",
                                          Encoding::decode(f.first),
                                          cd->member_id);
                        }
                    }
                    for (const auto &f : cd->canvas2d) {
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Canvas2D>{
                                {}, cd->member_id, f.first});
                            logger->trace("send canvas2d_entry {} of member {}",
                                          Encoding::decode(f.first),
                                          cd->member_id);
                        }
                    }
                    for (const auto &f : cd->view) {
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::View>{
                                {}, cd->member_id, f.first});
                            logger->trace("send view_entry {} of member {}",
                                          Encoding::decode(f.first),
                                          cd->member_id);
                        }
                    }
                    for (const auto &f : cd->image) {
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(webcface::Message::Entry<
                                       webcface::Message::Image>{
                                {}, cd->member_id, f.first});
                            logger->trace("send image_entry {} of member {}",
                                          Encoding::decode(f.first),
                                          cd->member_id);
                        }
                    }
                    for (const auto &f : cd->func) {
                        if (!f.first.starts_with(field_separator)) {
                            this->pack(*f.second);
                            logger->trace("send func_info {} of member {}",
                                          Encoding::decode(f.second->field),
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
                v.caller_id, v.target_member_id, Encoding::decode(v.field),
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
                                      Encoding::decode(v.field), v.args.size());
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
                logger->debug("value {} = {}", Encoding::decode(v.field),
                              (*v.data)[0]);
            } else {
                logger->debug("value {} = (array length = {})",
                              Encoding::decode(v.field), v.data->size());
            }
            if (!this->value.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::Value>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send value_entry {} of member {}",
                                          Encoding::decode(v.field),
                                          this->member_id);
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
                                      Encoding::decode(sub_field));
                }
            });
            break;
        }
        case MessageKind::text: {
            auto v = std::any_cast<webcface::Message::Text>(obj);
            logger->debug("text {} = {}", Encoding::decode(v.field),
                          static_cast<std::string>(*v.data));
            if (!this->text.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::Text>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send text_entry {} of member {}",
                                          Encoding::decode(v.field),
                                          this->member_id);
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
                                      Encoding::decode(sub_field));
                }
            });
            break;
        }
        case MessageKind::robot_model: {
            auto v = std::any_cast<webcface::Message::RobotModel>(obj);
            logger->debug("robot model {}", Encoding::decode(v.field));
            if (!this->robot_model.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::Message::Entry<
                                 webcface::Message::RobotModel>{
                            {}, this->member_id, v.field});
                        cd->logger->trace(
                            "send robot_model_entry {} of member {}",
                            Encoding::decode(v.field), this->member_id);
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
                                      req_id, Encoding::decode(sub_field));
                }
            });
            break;
        }
        case MessageKind::view: {
            auto v = std::any_cast<webcface::Message::View>(obj);
            logger->debug("view {} diff={}, length={}",
                          Encoding::decode(v.field), v.data_diff->size(),
                          v.length);
            if (!this->view.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::View>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send view_entry {} of member {}",
                                          Encoding::decode(v.field),
                                          this->member_id);
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
                                      Encoding::decode(sub_field));
                }
            });
            break;
        }
        case MessageKind::canvas3d: {
            auto v = std::any_cast<webcface::Message::Canvas3D>(obj);
            logger->debug("canvas3d {} diff={}, length={}",
                          Encoding::decode(v.field), v.data_diff->size(),
                          v.length);
            if (!this->canvas3d.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::Message::Entry<
                                 webcface::Message::Canvas3D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas3d_entry {} of member {}",
                                          Encoding::decode(v.field),
                                          this->member_id);
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
                                      req_id, Encoding::decode(sub_field));
                }
            });
            break;
        }
        case MessageKind::canvas2d: {
            auto v = std::any_cast<webcface::Message::Canvas2D>(obj);
            logger->debug("canvas2d {} diff={}, length={}",
                          Encoding::decode(v.field), v.data_diff->size(),
                          v.length);
            if (!this->canvas2d.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(webcface::Message::Entry<
                                 webcface::Message::Canvas2D>{
                            {}, this->member_id, v.field});
                        cd->logger->trace("send canvas2d_entry {} of member {}",
                                          Encoding::decode(v.field),
                                          this->member_id);
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
                                      req_id, Encoding::decode(sub_field));
                }
            });
            break;
        }
        case MessageKind::image: {
            auto v = std::any_cast<webcface::Message::Image>(obj);
            logger->debug("image {} ({} x {} x {})", Encoding::decode(v.field),
                          v.rows(), v.cols(), v.channels());
            if (!this->image.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->name != this->name) {
                        cd->pack(
                            webcface::Message::Entry<webcface::Message::Image>{
                                {}, this->member_id, v.field});
                        cd->logger->trace("send image_entry {} of member {}",
                                          Encoding::decode(v.field),
                                          this->member_id);
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
            logger->debug("func_info {}", Encoding::decode(v.field));
            if (!this->func.count(v.field) &&
                !v.field.starts_with(field_separator)) {
                store->forEach([&](auto cd) {
                    if (cd->member_id != this->member_id) {
                        cd->pack(v);
                        cd->logger->trace("send func_info {} of member {}",
                                          Encoding::decode(v.field),
                                          this->member_id);
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
                          Encoding::decode(s.field),
                          Encoding::decode(s.member));
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->value) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + field_separator)) {
                        std::u8string sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Value>{
                                s.req_id, sub_field, it.second});
                        logger->trace("send value_res req_id={} + '{}'",
                                      s.req_id, Encoding::decode(sub_field));
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
                          Encoding::decode(s.field),
                          Encoding::decode(s.member));
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->text) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + field_separator)) {
                        std::u8string sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Text>{
                                s.req_id, sub_field, it.second});
                        logger->trace("send text_res {}, req_id={} + '{}'",
                                      static_cast<std::string>(*it.second),
                                      s.req_id, Encoding::decode(sub_field));
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
                          Encoding::decode(s.field),
                          Encoding::decode(s.member));
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->robot_model) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + field_separator)) {
                        std::u8string sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(webcface::Message::Res<
                                   webcface::Message::RobotModel>{
                            s.req_id, sub_field, it.second});
                        logger->trace("send robot_model_res, req_id={} + '{}'",
                                      s.req_id, Encoding::decode(sub_field));
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
                          Encoding::decode(s.field),
                          Encoding::decode(s.member));
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->view) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            webcface::Message::View::ViewComponent>>();
                        for (std::size_t i = 0; i < it.second.size(); i++) {
                            diff->emplace(std::to_string(i), it.second[i]);
                        }
                        std::u8string sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::View>{
                                s.req_id, sub_field, diff, it.second.size()});
                        logger->trace("send view_res req_id={} + '{}'",
                                      s.req_id, Encoding::decode(sub_field));
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
                          Encoding::decode(s.field),
                          Encoding::decode(s.member));
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas3d) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            webcface::Message::Canvas3D::Canvas3DComponent>>();
                        for (std::size_t i = 0; i < it.second.size(); i++) {
                            diff->emplace(std::to_string(i), it.second[i]);
                        }
                        std::u8string sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Canvas3D>{
                                s.req_id, sub_field, diff, it.second.size()});
                        logger->trace("send canvas3d_res req_id={} + '{}'",
                                      s.req_id, Encoding::decode(sub_field));
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
                          Encoding::decode(s.field),
                          Encoding::decode(s.member));
            // 指定した値を返す
            store->findAndDo(s.member, [&](auto cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(webcface::Message::Sync{cd->member_id,
                                                       cd->last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd->canvas2d) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + field_separator)) {
                        auto diff = std::make_shared<std::unordered_map<
                            std::string,
                            webcface::Message::Canvas2D::Canvas2DComponent>>();
                        for (std::size_t i = 0; i < it.second.components.size();
                             i++) {
                            diff->emplace(std::to_string(i),
                                          it.second.components[i]);
                        }
                        std::u8string sub_field;
                        if (it.first == s.field) {
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            webcface::Message::Res<webcface::Message::Canvas2D>{
                                s.req_id, sub_field, it.second.width,
                                it.second.height, diff,
                                it.second.components.size()});
                        logger->trace("send canvas2d_res req_id={} + '{}'",
                                      s.req_id, Encoding::decode(sub_field));
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
                          s.req_id, Encoding::decode(s.field),
                          Encoding::decode(s.member), s.rows.value_or(-1),
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
            auto s = std::any_cast<webcface::Message::LogReq>(obj);
            logger->debug("request log from {}", Encoding::decode(s.member));
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

/*!
 * \brief cdの画像を変換しthisに送信
 *
 * cd.image[field]が更新されるかリクエストが更新されたときに変換を行う。
 *
 */
void MemberData::imageConvertThreadMain(const std::u8string &member,
                                        const std::u8string &field) {
#if !WEBCFACE_USE_OPENCV
    static bool opencv_warned = false;
#endif
    auto member_s = Encoding::decode(member);
    auto field_s = Encoding::decode(field);
    int last_image_flag = -1, last_req_flag = -1;
    logger->trace("imageConvertThreadMain started for {}, {}", member_s,
                  field_s);
    while (true) {
        store->findAndDo(member, [&](auto cd) {
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
                    logger->trace("converting image of {}, {}", member_s,
                                  field_s);
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
                auto sync =
                    webcface::Message::Sync{cd->member_id, cd->last_sync_time};

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
                    if(!opencv_warned){
                        this->logger->warn("Cannot convert image since OpenCV is disabled.");
                        opencv_warned = true;
                    }
                    return;
#endif
                }
                if (info.color_mode && *info.color_mode != img.color_mode()) {
#if WEBCFACE_USE_OPENCV
                    cv::cvtColor(
                        m, m, colorConvert(img.color_mode(), *info.color_mode));
#else
                    if(!opencv_warned){
                        this->logger->warn("Cannot convert image since OpenCV is disabled.");
                        opencv_warned = true;
                    }
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
                    if(!opencv_warned){
                        this->logger->warn("Cannot convert image since OpenCV is disabled.");
                        opencv_warned = true;
                    }
                    return;
#endif
                }
                Common::ImageBase img_send{
                    rows, cols, encoded,
                    info.color_mode.value_or(img.color_mode()), info.cmp_mode};

                while (!cd->closing.load() && !this->closing.load()) {
                    if (store->server->server_mtx.try_lock()) {
                        this->pack(sync);
                        this->pack(
                            webcface::Message::Res<webcface::Message::Image>{
                                req_id, sub_field, img_send});
                        logger->trace("send image_res req_id={} + '{}'", req_id,
                                      Encoding::decode(sub_field));
                        this->send();
                        store->server->server_mtx.unlock();
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
} // namespace Server
WEBCFACE_NS_END

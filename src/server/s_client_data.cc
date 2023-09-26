#include "s_client_data.h"
#include "store.h"
#include "../message/message.h"
#include "../include/webcface/common/def.h"
#include <cinatra.hpp>
#include <algorithm>
#include <iterator>

namespace WebCFace::Server {
void ClientData::onClose() {
    // 作ったものの何もすることがなかった
    logger->info("connection closed");
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
        std::static_pointer_cast<cinatra::connection<cinatra::NonSSL>>(con)
            ->send_ws_binary(msg);
    }
}
bool ClientData::connected() const {
    return con &&
           !std::static_pointer_cast<cinatra::connection<cinatra::NonSSL>>(con)
                ->has_close();
}
void ClientData::onConnect() { logger->info("connected"); }

bool ClientData::hasReq(const std::string &member) {
    return std::any_of(this->value_req[member].begin(),
                       this->value_req[member].end(),
                       [](const auto &it) { return it.second > 0; }) ||
           std::any_of(this->text_req[member].begin(),
                       this->text_req[member].end(),
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
    namespace MessageKind = WebCFace::Message::MessageKind;
    auto messages = WebCFace::Message::unpack(message, this->logger);
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::ping: {
            this->last_ping_duration =
                std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now() - this->last_send_ping);
            break;
        }
        case MessageKind::ping_status_req: {
            this->ping_status_req = true;
            break;
        }
        case MessageKind::sync_init: {
            auto v = std::any_cast<WebCFace::Message::SyncInit>(obj);
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
            v.addr =
                std::static_pointer_cast<cinatra::connection<cinatra::NonSSL>>(
                    con)
                    ->remote_address();
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
                // 全クライアントに新しいMemberを通知
                store.forEach([&](auto &cd) {
                    if (cd.member_id != this->member_id) {
                        cd.pack(v);
                        cd.logger->trace("send sync_init {} ({})", this->name,
                                         this->member_id);
                    }
                });
            }
            this->pack(WebCFace::Message::SvrVersion{
                {}, WEBCFACE_SERVER_NAME, WEBCFACE_VERSION});
            // 逆に新しいMemberに他の全Memberのentryを通知
            store.forEachWithName([&](auto &cd) {
                if (cd.member_id != this->member_id) {
                    this->pack(cd.init_data);
                    logger->trace("send sync_init {} ({})", cd.name,
                                  cd.member_id);

                    for (const auto &f : cd.value) {
                        this->pack(
                            WebCFace::Message::Entry<WebCFace::Message::Value>{
                                {}, cd.member_id, f.first});
                        logger->trace("send value_entry {} of member {}",
                                      f.first, cd.member_id);
                    }
                    for (const auto &f : cd.text) {
                        this->pack(
                            WebCFace::Message::Entry<WebCFace::Message::Text>{
                                {}, cd.member_id, f.first});
                        logger->trace("send text_entry {} of member {}",
                                      f.first, cd.member_id);
                    }
                    for (const auto &f : cd.view) {
                        this->pack(
                            WebCFace::Message::Entry<WebCFace::Message::View>{
                                {}, cd.member_id, f.first});
                        logger->trace("send view_entry {} of member {}",
                                      f.first, cd.member_id);
                    }
                    for (const auto &f : cd.func) {
                        this->pack(*f.second);
                        logger->trace("send func_info {} of member {}",
                                      f.second->field, cd.member_id);
                    }
                }
            });
            break;
        }
        case MessageKind::sync: {
            auto v = std::any_cast<WebCFace::Message::Sync>(obj);
            v.member_id = this->member_id;
            logger->debug("sync");
            // 1つ以上リクエストしているクライアントにはsyncの情報を流す
            store.forEach([&](auto &cd) {
                if (cd.hasReq(this->name)) {
                    cd.pack(v);
                    cd.logger->trace("send sync {}", this->member_id);
                }
            });
            break;
        }
        case MessageKind::call: {
            auto v = std::any_cast<WebCFace::Message::Call>(obj);
            v.caller_member_id = this->member_id;
            logger->debug(
                "call caller_id={}, target_id={}, field={}, with {} args",
                v.caller_id, v.target_member_id, v.field, v.args.size());
            // そのままターゲットのクライアントに送る
            store.findAndDo(
                v.target_member_id,
                [&](auto &cd) {
                    cd.pack(v);
                    cd.logger->trace("send call caller_id={}, target_id={}, "
                                     "field={}, with {} args",
                                     v.caller_id, v.target_member_id, v.field,
                                     v.args.size());
                },
                [&]() {
                    // 関数存在しないときの処理
                    this->pack(WebCFace::Message::CallResponse{
                        {}, v.caller_id, v.caller_member_id, false});
                    logger->debug("call target not found");
                });
            break;
        }
        case MessageKind::call_response: {
            auto v = std::any_cast<WebCFace::Message::CallResponse>(obj);
            logger->debug("call_response to (member_id {}, caller_id {}), {}",
                          v.caller_member_id, v.caller_id, v.started);
            // そのままcallerに送る
            store.findAndDo(v.caller_member_id, [&](auto &cd) {
                cd.pack(v);
                cd.logger->trace(
                    "send call_response to (member_id {}, caller_id {}), {}",
                    v.caller_member_id, v.caller_id, v.started);
            });
            break;
        }
        case MessageKind::call_result: {
            auto v = std::any_cast<WebCFace::Message::CallResult>(obj);
            logger->debug("call_result to (member_id {}, caller_id {}), {}",
                          v.caller_member_id, v.caller_id,
                          static_cast<std::string>(v.result));
            // そのままcallerに送る
            store.findAndDo(v.caller_member_id, [&](auto &cd) {
                cd.pack(v);
                cd.logger->trace(
                    "send call_result to (member_id {}, caller_id {}), {}",
                    v.caller_member_id, v.caller_id,
                    static_cast<std::string>(v.result));
            });
            break;
        }
        case MessageKind::value: {
            auto v = std::any_cast<WebCFace::Message::Value>(obj);
            if (v.data->size() == 1) {
                logger->debug("value {} = {}", v.field, (*v.data)[0]);
            } else {
                logger->debug("value {} = (array length = {})", v.field,
                              v.data->size());
            }
            if (!this->value.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(
                            WebCFace::Message::Entry<WebCFace::Message::Value>{
                                {}, this->member_id, v.field});
                        cd.logger->trace("send value_entry {} of member {}",
                                         v.field, this->member_id);
                    }
                });
            }
            this->value[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                auto [req_id, sub_field] =
                    findReqField(cd.value_req, this->name, v.field);
                if (req_id > 0) {
                    cd.pack(WebCFace::Message::Res<WebCFace::Message::Value>(
                        req_id, sub_field, v.data));
                    cd.logger->trace("send value_res req_id={} + '{}'", req_id,
                                     sub_field);
                }
            });
            break;
        }
        case MessageKind::text: {
            auto v = std::any_cast<WebCFace::Message::Text>(obj);
            logger->debug("text {} = {}", v.field, *v.data);
            if (!this->text.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(
                            WebCFace::Message::Entry<WebCFace::Message::Text>{
                                {}, this->member_id, v.field});
                        cd.logger->trace("send text_entry {} of member {}",
                                         v.field, this->member_id);
                    }
                });
            }
            this->text[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                auto [req_id, sub_field] =
                    findReqField(cd.text_req, this->name, v.field);
                if (req_id > 0) {
                    cd.pack(WebCFace::Message::Res<WebCFace::Message::Text>(
                        req_id, sub_field, v.data));
                    cd.logger->trace("send text_res {}, req_id={} + '{}'",
                                     *v.data, req_id, sub_field);
                }
            });
            break;
        }
        case MessageKind::view: {
            auto v = std::any_cast<WebCFace::Message::View>(obj);
            logger->debug("view {} diff={}, length={}", v.field,
                          v.data_diff->size(), v.length);
            if (!this->view.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(
                            WebCFace::Message::Entry<WebCFace::Message::View>{
                                {}, this->member_id, v.field});
                        cd.logger->trace("send view_entry {} of member {}",
                                         v.field, this->member_id);
                    }
                });
            }
            this->view[v.field].resize(v.length);
            for (const auto &d : *v.data_diff) {
                this->view[v.field][d.first] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                auto [req_id, sub_field] =
                    findReqField(cd.view_req, this->name, v.field);
                if (req_id > 0) {
                    cd.pack(WebCFace::Message::Res<WebCFace::Message::View>(
                        req_id, sub_field, v.data_diff, v.length));
                    cd.logger->trace("send view_res req_id={} + '{}'", req_id,
                                     sub_field);
                }
            });
            break;
        }
        case MessageKind::log: {
            auto v = std::any_cast<WebCFace::Message::Log>(obj);
            v.member_id = this->member_id;
            logger->debug("log {} lines", v.log->size());
            std::copy(v.log->begin(), v.log->end(),
                      std::back_inserter(*this->log));
            // このlogをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                if (cd.log_req.count(this->name)) {
                    cd.pack(v);
                    cd.logger->trace("send log {} lines", v.log->size());
                }
            });
            break;
        }
        case MessageKind::func_info: {
            auto v = std::any_cast<WebCFace::Message::FuncInfo>(obj);
            v.member_id = this->member_id;
            logger->debug("func_info {}", v.field);
            if (!this->func.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.member_id != this->member_id) {
                        cd.pack(v);
                        cd.logger->trace("send func_info {} of member {}",
                                         v.field, this->member_id);
                    }
                });
            }
            this->func[v.field] = std::make_shared<Message::FuncInfo>(v);
            break;
        }
        case MessageKind::req + MessageKind::value: {
            auto s =
                std::any_cast<WebCFace::Message::Req<WebCFace::Message::Value>>(
                    obj);
            logger->debug("request value ({}): {} from {}", s.req_id, s.field,
                          s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WebCFace::Message::Sync{cd.member_id,
                                                       cd.last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd.value) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            WebCFace::Message::Res<WebCFace::Message::Value>{
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
            auto s =
                std::any_cast<WebCFace::Message::Req<WebCFace::Message::Text>>(
                    obj);
            logger->debug("request text ({}): {} from {}", s.req_id, s.field,
                          s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WebCFace::Message::Sync{cd.member_id,
                                                       cd.last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd.text) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            WebCFace::Message::Res<WebCFace::Message::Text>{
                                s.req_id, sub_field, it.second});
                        logger->trace("send text_res {}, req_id={} + '{}'",
                                      *it.second, s.req_id, sub_field);
                    }
                }
            });
            text_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::view: {
            auto s =
                std::any_cast<WebCFace::Message::Req<WebCFace::Message::View>>(
                    obj);
            logger->debug("request view ({}): {} from {}", s.req_id, s.field,
                          s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                if (!this->hasReq(s.member)) {
                    this->pack(WebCFace::Message::Sync{cd.member_id,
                                                       cd.last_sync_time});
                    logger->trace("send sync {}", this->member_id);
                }
                for (const auto &it : cd.view) {
                    if (it.first == s.field ||
                        it.first.starts_with(s.field + ".")) {
                        auto diff = std::make_shared<std::unordered_map<
                            int, WebCFace::Message::View::ViewComponent>>();
                        for (std::size_t i = 0; i < it.second.size(); i++) {
                            diff->emplace(i, it.second[i]);
                        }
                        std::string sub_field;
                        if (it.first == s.field) {
                            sub_field = "";
                        } else {
                            sub_field = it.first.substr(s.field.size() + 1);
                        }
                        this->pack(
                            WebCFace::Message::Res<WebCFace::Message::View>{
                                s.req_id, sub_field, diff, it.second.size()});
                        logger->trace("send view_res req_id={} + '{}'",
                                      s.req_id, sub_field);
                    }
                }
            });
            view_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::log_req: {
            auto s = std::any_cast<WebCFace::Message::LogReq>(obj);
            logger->debug("request log from {}", s.member);
            std::cout << this->name << ": request log " << s.member
                      << std::endl;
            log_req.insert(s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                this->pack(WebCFace::Message::Log{{}, cd.member_id, cd.log});
                logger->trace("send log {} lines", cd.log->size());
            });
            break;
        }
        case MessageKind::entry + MessageKind::value:
        case MessageKind::res + MessageKind::value:
        case MessageKind::entry + MessageKind::text:
        case MessageKind::res + MessageKind::text:
        case MessageKind::entry + MessageKind::view:
        case MessageKind::res + MessageKind::view:
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
} // namespace WebCFace::Server

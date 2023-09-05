#include "s_client_data.h"
#include "store.h"
#include "../message/message.h"
#include <cinatra.hpp>
#include <algorithm>
#include <iostream>
#include <iterator>

namespace WebCFace::Server {
void ClientData::onClose() {
    // 作ったものの何もすることがなかった
}
void ClientData::send() {
    if (connected() && send_len > 0) {
        std::static_pointer_cast<cinatra::connection<cinatra::NonSSL>>(con)
            ->send_ws_binary(Message::packDone(send_buffer, send_len));
    }
    send_buffer.str("");
    send_len = 0;
}
bool ClientData::connected() const {
    return con &&
           !std::static_pointer_cast<cinatra::connection<cinatra::NonSSL>>(con)
                ->has_close();
}
void ClientData::onConnect() {}

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

void ClientData::onRecv(const std::string &message) {
    namespace MessageKind = WebCFace::Message::MessageKind;
    auto messages = WebCFace::Message::unpack(message);
    for (const auto &m : messages) {
        const auto &[kind, obj] = m;
        switch (kind) {
        case MessageKind::sync_init: {
            auto v = std::any_cast<WebCFace::Message::SyncInit>(obj);
            this->name = v.member;
            this->sync_init = true;
            if (v.member == "") {
                std::cout << "anonymous client connected" << std::endl;
            } else {
                std::cout << this->name << ": connected" << std::endl;
                store.clients_by_name.erase(this->name);
                store.clients_by_name.emplace(this->name, store.getClient(con));
                // 全クライアントに新しいMemberを通知
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(v);
                    }
                });
            }
            // 逆に新しいMemberに他の全Memberのentryを通知
            store.forEachWithName([&](auto &cd) {
                if (cd.name != this->name) {
                    this->pack(WebCFace::Message::SyncInit{{}, cd.name});

                    for (const auto &f : cd.value) {
                        this->pack(
                            WebCFace::Message::Entry<WebCFace::Message::Value>{
                                {}, cd.name, f.first});
                    }
                    for (const auto &f : cd.text) {
                        this->pack(
                            WebCFace::Message::Entry<WebCFace::Message::Text>{
                                {}, cd.name, f.first});
                    }
                    for (const auto &f : cd.view) {
                        this->pack(
                            WebCFace::Message::Entry<WebCFace::Message::View>{
                                {}, cd.name, f.first});
                    }
                    for (const auto &f : cd.func) {
                        this->pack(f.second);
                    }
                }
            });
            break;
        }
        case MessageKind::sync: {
            auto v = std::any_cast<WebCFace::Message::Sync>(obj);
            v.member = this->name;
            // 1つ以上リクエストしているクライアントにはsyncの情報を流す
            store.forEach([&](auto &cd) {
                if (cd.hasReq(this->name)) {
                    cd.pack(v);
                }
            });
            break;
        }
        case MessageKind::call: {
            auto v = std::any_cast<WebCFace::Message::Call>(obj);
            v.caller = this->name;
            std::cout << this->name << ": call [" << v.caller_id << "] "
                      << v.member << ":" << v.field << " (args = ";
            for (const auto &a : v.args) {
                std::cout << static_cast<std::string>(a) << ", ";
            }
            std::cout << ")" << std::endl;
            // そのままターゲットのクライアントに送る
            store.findAndDo(
                v.member, [&](auto &cd) { cd.pack(v); },
                [&]() {
                    // 関数存在しないときの処理
                    this->pack(WebCFace::Message::CallResponse{
                        {}, v.caller_id, v.caller, false});
                });
            break;
        }
        case MessageKind::call_response: {
            auto v = std::any_cast<WebCFace::Message::CallResponse>(obj);
            std::cout << this->name << ": call response [" << v.caller_id
                      << "] " << v.started << std::endl;
            // そのままcallerに送る
            store.findAndDo(v.caller, [&](auto &cd) { cd.pack(v); });
            break;
        }
        case MessageKind::call_result: {
            auto v = std::any_cast<WebCFace::Message::CallResult>(obj);
            std::cout << this->name << ": call result [" << v.caller_id << "] '"
                      << static_cast<std::string>(v.result) << "'";
            if (v.is_error) {
                std::cout << "(error)";
            }
            std::cout << std::endl;
            // そのままcallerに送る
            store.findAndDo(v.caller, [&](auto &cd) { cd.pack(v); });
            break;
        }
        case MessageKind::value: {
            auto v = std::any_cast<WebCFace::Message::Value>(obj);
            std::cout << this->name << ": value " << v.field << " = " << v.data
                      << ", send back to ";
            if (!this->value.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(
                            WebCFace::Message::Entry<WebCFace::Message::Value>{
                                {}, this->name, v.field});
                    }
                });
            }
            this->value[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                int req_id = cd.value_req[this->name][v.field];
                if (req_id > 0) {
                    cd.pack(WebCFace::Message::Res<WebCFace::Message::Value>(
                        req_id, v.data));
                    std::cout << cd.name << "(" << req_id << "), ";
                }
            });
            std::cout << std::endl;
            break;
        }
        case MessageKind::text: {
            auto v = std::any_cast<WebCFace::Message::Text>(obj);
            std::cout << this->name << ": text " << v.field << " = " << v.data
                      << ", send back to ";
            if (!this->text.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(
                            WebCFace::Message::Entry<WebCFace::Message::Text>{
                                {}, this->name, v.field});
                    }
                });
            }
            this->text[v.field] = v.data;
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                int req_id = cd.text_req[this->name][v.field];
                if (req_id > 0) {
                    cd.pack(WebCFace::Message::Res<WebCFace::Message::Text>(
                        req_id, v.data));
                    std::cout << cd.name << "(" << req_id << "), ";
                }
            });
            std::cout << std::endl;
            break;
        }
        case MessageKind::view: {
            auto v = std::any_cast<WebCFace::Message::View>(obj);
            std::cout << this->name << ": view " << v.field
                      << " diff = " << v.data_diff.size()
                      << ", length = " << v.length << ", send back to ";
            if (!this->view.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(
                            WebCFace::Message::Entry<WebCFace::Message::View>{
                                {}, this->name, v.field});
                    }
                });
            }
            this->view[v.field].resize(v.length);
            for (const auto &d : v.data_diff) {
                this->view[v.field][d.first] = d.second;
            }
            // このvalueをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                int req_id = cd.view_req[this->name][v.field];
                if (req_id > 0) {
                    cd.pack(WebCFace::Message::Res<WebCFace::Message::View>(
                        req_id, v.data_diff, v.length));
                    std::cout << cd.name << "(" << req_id << "), ";
                }
            });
            std::cout << std::endl;
            break;
        }
        case MessageKind::log: {
            auto v = std::any_cast<WebCFace::Message::Log>(obj);
            v.member = this->name;
            std::cout << this->name << ": log " << v.log.size() << " lines"
                      << ", send back to ";
            std::copy(v.log.begin(), v.log.end(),
                      std::back_inserter(this->log));
            // このlogをsubscribeしてるところに送り返す
            store.forEach([&](auto &cd) {
                if (cd.log_req.count(this->name)) {
                    cd.pack(v);
                    std::cout << cd.name << ", ";
                }
            });
            std::cout << std::endl;
            break;
        }
        case MessageKind::func_info: {
            auto v = std::any_cast<WebCFace::Message::FuncInfo>(obj);
            v.member = this->name;
            std::cout << this->name << ": func_info " << v.field << " arg: ";
            for (std::size_t i = 0; i < v.args.size(); i++) {
                if (i > 0) {
                    std::cout << ", ";
                }
                std::cout << static_cast<WebCFace::Arg>(v.args[i]);
            }
            std::cout << " ret: " << static_cast<ValType>(v.return_type)
                      << std::endl;
            if (!this->func.count(v.field)) {
                store.forEach([&](auto &cd) {
                    if (cd.name != this->name) {
                        cd.pack(v);
                    }
                });
            }
            this->func[v.field] = v;
            break;
        }
        case MessageKind::req + MessageKind::value: {
            auto s =
                std::any_cast<WebCFace::Message::Req<WebCFace::Message::Value>>(
                    obj);
            std::cout << this->name << ": request value " << s.member << ":"
                      << s.field << " (" << s.req_id << ")" << std::endl;
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                auto it = cd.value.find(s.field);
                if (it != cd.value.end()) {
                    if (!this->hasReq(s.member)) {
                        this->pack(WebCFace::Message::Sync{s.member,
                                                           cd.last_sync_time});
                    }
                    this->pack(WebCFace::Message::Res<WebCFace::Message::Value>{
                        s.req_id, it->second});
                }
            });
            value_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::text: {
            auto s =
                std::any_cast<WebCFace::Message::Req<WebCFace::Message::Text>>(
                    obj);
            std::cout << this->name << ": request text " << s.member << ":"
                      << s.field << std::endl;
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                auto it = cd.text.find(s.field);
                if (it != cd.text.end()) {
                    if (!this->hasReq(s.member)) {
                        this->pack(WebCFace::Message::Sync{s.member,
                                                           cd.last_sync_time});
                    }
                    this->pack(WebCFace::Message::Res<WebCFace::Message::Text>{
                        s.req_id, it->second});
                }
            });
            text_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::req + MessageKind::view: {
            auto s =
                std::any_cast<WebCFace::Message::Req<WebCFace::Message::View>>(
                    obj);
            std::cout << this->name << ": request view " << s.member << ":"
                      << s.field << std::endl;
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                auto it = cd.view.find(s.field);
                if (it != cd.view.end()) {
                    if (!this->hasReq(s.member)) {
                        this->pack(WebCFace::Message::Sync{s.member,
                                                           cd.last_sync_time});
                    }
                    std::unordered_map<int,
                                       WebCFace::Message::View::ViewComponent>
                        diff;
                    for (std::size_t i = 0; i < it->second.size(); i++) {
                        diff[i] = it->second[i];
                    }
                    this->pack(WebCFace::Message::Res<WebCFace::Message::View>{
                        s.req_id, diff, static_cast<int>(it->second.size())});
                }
            });
            view_req[s.member][s.field] = s.req_id;
            break;
        }
        case MessageKind::log_req: {
            auto s = std::any_cast<WebCFace::Message::LogReq>(obj);
            std::cout << this->name << ": request log " << s.member
                      << std::endl;
            log_req.insert(s.member);
            // 指定した値を返す
            store.findAndDo(s.member, [&](auto &cd) {
                this->pack(WebCFace::Message::Log{{}, s.member, cd.log});
            });
            break;
        }
        case MessageKind::entry + MessageKind::value:
        case MessageKind::res + MessageKind::value:
        case MessageKind::entry + MessageKind::text:
        case MessageKind::res + MessageKind::text:
        case MessageKind::entry + MessageKind::view:
        case MessageKind::res + MessageKind::view:
            std::cerr << "Invalid Message Kind " << static_cast<int>(kind)
                      << std::endl;
            break;
        default:
            std::cerr << "Unknown Message Kind " << static_cast<int>(kind)
                      << std::endl;
            break;
        }
    }
    store.clientSendAll();
}
} // namespace WebCFace::Server

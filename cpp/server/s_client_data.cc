#include "s_client_data.h"
#include "store.h"
#include "../message/message.h"
#include <algorithm>
#include <iostream>
#include <iterator>

namespace WebCFace::Server {
void ClientData::onClose() {
    // 作ったものの何もすることがなかった
}
void ClientData::send(const std::vector<char> &m) const {
    if (connected()) {
        con->send(&m[0], m.size(), drogon::WebSocketMessageType::Binary);
    }
}
bool ClientData::connected() const { return con && con->connected(); }
void ClientData::onConnect() {}

void ClientData::onRecv(const std::string &message) {
    using MessageKind = WebCFace::Message::MessageKind;
    auto [kind, obj] = WebCFace::Message::unpack(message);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
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
            for (const auto &c : store.clients) {
                if (c.second->sync_init && c.second->name != this->name &&
                    this->name != "") {
                    c.second->send(WebCFace::Message::pack(v));
                }
            }
        }
        // 逆に新しいMemberに他の全Memberのentryを通知
        for (const auto &c : store.clients) {
            if (c.second->sync_init && c.second->name != this->name &&
                c.second->name != "") {
                this->send(WebCFace::Message::pack(
                    WebCFace::Message::SyncInit{{}, c.second->name}));

                for (const auto &p : c.second->value) {
                    this->send(WebCFace::Message::pack(
                        WebCFace::Message::Entry<WebCFace::Message::Value>{
                            {}, c.second->name, p.first}));
                }
                for (const auto &p : c.second->text) {
                    this->send(WebCFace::Message::pack(
                        WebCFace::Message::Entry<WebCFace::Message::Text>{
                            {}, c.second->name, p.first}));
                }
                for (const auto &p : c.second->func) {
                    this->send(WebCFace::Message::pack(p.second));
                }
            }
        }
        break;
    }
    case MessageKind::call: {
        auto v = std::any_cast<WebCFace::Message::Call>(obj);
        v.caller = this->name;
        std::cout << this->name << ": call [" << v.caller_id << "] " << v.member
                  << ":" << v.name << " (args = ";
        for (const auto &a : v.args) {
            std::cout << a << ", ";
        }
        std::cout << ")" << std::endl;
        // そのままターゲットのクライアントに送る
        auto c_it = store.clients_by_name.find(v.member);
        if (c_it != store.clients_by_name.end()) {
            c_it->second->send(WebCFace::Message::pack(v));
        } else {
            // 関数存在しないときの処理
            this->send(WebCFace::Message::pack(WebCFace::Message::CallResponse{
                {}, v.caller_id, v.caller, false}));
        }
        break;
    }
    case MessageKind::call_response: {
        auto v = std::any_cast<WebCFace::Message::CallResponse>(obj);
        std::cout << this->name << ": call response [" << v.caller_id << "] "
                  << v.started << std::endl;
        // そのままcallerに送る
        auto c_it = store.clients_by_name.find(v.caller);
        if (c_it != store.clients_by_name.end()) {
            c_it->second->send(WebCFace::Message::pack(v));
        }
        break;
    }
    case MessageKind::call_result: {
        auto v = std::any_cast<WebCFace::Message::CallResult>(obj);
        std::cout << this->name << ": call result [" << v.caller_id << "] '"
                  << v.result << "'";
        if (v.is_error) {
            std::cout << "(error)";
        }
        std::cout << std::endl;
        // そのままcallerに送る
        auto c_it = store.clients_by_name.find(v.caller);
        if (c_it != store.clients_by_name.end()) {
            c_it->second->send(WebCFace::Message::pack(v));
        }
        break;
    }
    case MessageKind::value: {
        auto v = std::any_cast<WebCFace::Message::Value>(obj);
        v.member = this->name;
        std::cout << this->name << ": value " << v.name << " = " << v.data
                  << ", send back to ";
        if (!this->value.count(v.name)) {
            for (const auto &c : store.clients) {
                if (c.second->sync_init && c.second->name != this->name) {
                    c.second->send(WebCFace::Message::pack(
                        WebCFace::Message::Entry<WebCFace::Message::Value>{
                            {}, v.member, v.name}));
                }
            }
        }
        this->value[v.name] = v.data;
        // このvalueをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            if (c.second->sync_init) {
                for (const auto &s : c.second->value_subsc) {
                    if (s.first == this->name && s.second == v.name) {
                        c.second->send(WebCFace::Message::pack(v));
                        std::cout << c.second->name << ", ";
                    }
                }
            }
        }
        std::cout << std::endl;
        break;
    }
    case MessageKind::text: {
        auto v = std::any_cast<WebCFace::Message::Text>(obj);
        v.member = this->name;
        std::cout << this->name << ": text " << v.name << " = " << v.data
                  << ", send back to ";
        if (!this->text.count(v.name)) {
            for (const auto &c : store.clients) {
                if (c.second->sync_init && c.second->name != this->name) {
                    c.second->send(WebCFace::Message::pack(
                        WebCFace::Message::Entry<WebCFace::Message::Text>{
                            {}, v.member, v.name}));
                }
            }
        }
        this->text[v.name] = v.data;
        // このvalueをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            if (c.second->sync_init) {
                for (const auto &s : c.second->text_subsc) {
                    if (s.first == this->name && s.second == v.name) {
                        c.second->send(WebCFace::Message::pack(v));
                        std::cout << c.second->name << ", ";
                    }
                }
            }
        }
        std::cout << std::endl;
        break;
    }
    case MessageKind::log: {
        auto v = std::any_cast<WebCFace::Message::Log>(obj);
        v.member = this->name;
        std::cout << this->name << ": log " << v.log.size() << " lines"
                  << ", send back to ";
        std::copy(v.log.begin(), v.log.end(), std::back_inserter(this->log));
        // このlogをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            if (c.second->sync_init) {
                for (const auto &s : c.second->log_subsc) {
                    if (s == this->name) {
                        c.second->send(WebCFace::Message::pack(v));
                        std::cout << c.second->name << ", ";
                    }
                }
            }
        }
        std::cout << std::endl;
        break;
    }
    case MessageKind::func_info: {
        auto v = std::any_cast<WebCFace::Message::FuncInfo>(obj);
        v.member = this->name;
        std::cout << this->name << ": func_info " << v.name << " arg: ";
        for (std::size_t i = 0; i < v.args.size(); i++) {
            if (i > 0) {
                std::cout << ", ";
            }
            std::cout << static_cast<WebCFace::Arg>(v.args[i]);
        }
        std::cout << " ret: " << static_cast<ValType>(v.return_type)
                  << std::endl;
        if (!this->func.count(v.name)) {
            for (const auto &c : store.clients) {
                if (c.second->sync_init && c.second->name != this->name) {
                    c.second->send(WebCFace::Message::pack(v));
                }
            }
        }
        this->func[v.name] = v;
        break;
    }
    case kind_subscribe(MessageKind::value): {
        auto s = std::any_cast<
            WebCFace::Message::Subscribe<WebCFace::Message::Value>>(obj);
        std::cout << this->name << ": subscribe value " << s.from << ":"
                  << s.name << std::endl;
        value_subsc.insert(std::make_pair(s.from, s.name));
        // 指定した値を返す
        auto c_it = store.clients_by_name.find(s.from);
        if (c_it != store.clients_by_name.end()) {
            auto it = c_it->second->value.find(s.name);
            if (it != c_it->second->value.end()) {
                this->send(WebCFace::Message::pack(
                    WebCFace::Message::Value{{}, s.from, s.name, it->second}));
            }
        }
        break;
    }
    case kind_subscribe(MessageKind::text): {
        auto s = std::any_cast<
            WebCFace::Message::Subscribe<WebCFace::Message::Text>>(obj);
        std::cout << this->name << ": subscribe text " << s.from << ":"
                  << s.name << std::endl;
        text_subsc.insert(std::make_pair(s.from, s.name));
        // 指定した値を返す
        auto c_it = store.clients_by_name.find(s.from);
        if (c_it != store.clients_by_name.end()) {
            auto it = c_it->second->text.find(s.name);
            if (it != c_it->second->text.end()) {
                this->send(WebCFace::Message::pack(
                    WebCFace::Message::Text{{}, s.from, s.name, it->second}));
            }
        }
        break;
    }
    case MessageKind::log_req: {
        auto s = std::any_cast<WebCFace::Message::LogReq>(obj);
        std::cout << this->name << ": subscribe log " << s.member << std::endl;
        log_subsc.insert(s.member);
        // 指定した値を返す
        auto c_it = store.clients_by_name.find(s.member);
        if (c_it != store.clients_by_name.end()) {
            this->send(WebCFace::Message::pack(
                WebCFace::Message::Log{{}, s.member, c_it->second->log}));
        }
        break;
    }
    case kind_entry(MessageKind::value):
    case kind_entry(MessageKind::text):
        std::cerr << "Invalid Message Kind " << static_cast<int>(kind)
                  << std::endl;
        break;
    default:
        std::cerr << "Unknown Message Kind " << static_cast<int>(kind)
                  << std::endl;
        break;
    }
#pragma GCC diagnostic pop
}
} // namespace WebCFace::Server

#include "s_client_data.h"
#include "store.h"
#include "../message/message.h"

#include <algorithm>
#include <iostream>

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
void ClientData::onConnect() {
    for (const auto &c : store.clients) {
        this->send(pack(c.second->entry));
    }
}
void ClientData::onRecv(const std::string &message) {
    bool entry_update = false;
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
    switch (kind) {
    case MessageKind::name:
        this->entry.name = std::any_cast<Name>(obj).name;
        std::cout << this->entry.name << ": connected" << std::endl;
        store.clients_by_name.erase(this->entry.name);
        store.clients_by_name.emplace(this->entry.name, store.getClient(con));
        break;
    case MessageKind::call: {
        auto v = std::any_cast<Call>(obj);
        v.caller = this->entry.name;
        std::cout << this->entry.name << ": call [" << v.caller_id << "] "
                  << v.receiver << ":" << v.name << " (args = ";
        for (const auto &a : v.args) {
            std::cout << a << ", ";
        }
        std::cout << ")" << std::endl;
        // そのままターゲットのクライアントに送る
        auto c_it = store.clients_by_name.find(v.receiver);
        if (c_it != store.clients_by_name.end()) {
            c_it->second->send(pack(v));
        } else {
            // 関数存在しないときの処理
            this->send(
                pack(CallResponse{{}, v.caller_id, v.caller, false, true, ""}));
        }
        break;
    }
    case MessageKind::call_response: {
        auto v = std::any_cast<CallResponse>(obj);
        std::cout << this->entry.name << ": call response [" << v.caller_id
                  << "] '" << v.response;
        if (v.is_error) {
            std::cout << "' (error)";
        }
        std::cout << std::endl;
        // そのままcallerに送る
        auto c_it = store.clients_by_name.find(v.caller);
        if (c_it != store.clients_by_name.end()) {
            c_it->second->send(pack(v));
        }
        break;
    }
    case MessageKind::value: {
        auto v = std::any_cast<Value>(obj);
        std::cout << this->entry.name << ": value " << v.name << " = " << v.data
                  << std::endl;
        value_history[v.name].push_back(v.data);
        bool has_entry =
            std::any_of(this->entry.value.begin(), this->entry.value.end(),
                        [&v](auto x) { return x.name == v.name; });
        if (!has_entry) {
            entry_update = true;
            this->entry.value.push_back({v.name});
        }
        // このvalueをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            for (const auto &s : c.second->value_subsc) {
                if (s.first == this->entry.name && s.second == v.name) {
                    c.second->send(pack(
                        Recv<Value>{{}, this->entry.name, v.name, v.data}));
                    std::cout << "update value " << this->entry.name << ":"
                              << v.name << " (= " << v.data << " ) -> "
                              << c.second->entry.name << std::endl;
                }
            }
        }
        break;
    }
    case MessageKind::text: {
        auto v = std::any_cast<Text>(obj);
        std::cout << this->entry.name << ": text " << v.name << " = " << v.data
                  << std::endl;
        text_history[v.name].push_back(v.data);
        bool has_entry =
            std::any_of(this->entry.text.begin(), this->entry.text.end(),
                        [&v](auto x) { return x.name == v.name; });
        if (!has_entry) {
            entry_update = true;
            this->entry.text.push_back({v.name});
        }
        // このvalueをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            for (const auto &s : c.second->text_subsc) {
                if (s.first == this->entry.name && s.second == v.name) {
                    c.second->send(
                        pack(Recv<Text>{{}, this->entry.name, v.name, v.data}));
                    std::cout << "update text " << this->entry.name << ":"
                              << v.name << " (= " << v.data << " ) -> "
                              << c.second->entry.name << std::endl;
                }
            }
        }
        break;
    }
    case kind_subscribe(MessageKind::value): {
        auto s = std::any_cast<Subscribe<Value>>(obj);
        std::cout << this->entry.name << ": subscribe value " << s.from << ":"
                  << s.name << std::endl;
        value_subsc.insert(std::make_pair(s.from, s.name));
        // 指定した値を返す
        auto c_it = store.clients_by_name.find(s.from);
        if (c_it != store.clients_by_name.end()) {
            auto it = c_it->second->value_history.find(s.name);
            if (it != c_it->second->value_history.end()) {
                this->send(
                    pack(Recv<Value>{{}, s.from, s.name, it->second.back()}));
                std::cout << "update value " << s.from << ":" << s.name
                          << " (= " << it->second.back() << " ) -> "
                          << this->entry.name << std::endl;
            }
        }
        break;
    }
    case kind_subscribe(MessageKind::text): {
        auto s = std::any_cast<Subscribe<Text>>(obj);
        std::cout << this->entry.name << ": subscribe text " << s.from << ":"
                  << s.name << std::endl;
        text_subsc.insert(std::make_pair(s.from, s.name));
        // 指定した値を返す
        auto c_it = store.clients_by_name.find(s.from);
        if (c_it != store.clients_by_name.end()) {
            auto it = c_it->second->text_history.find(s.name);
            if (it != c_it->second->text_history.end()) {
                this->send(
                    pack(Recv<Text>{{}, s.from, s.name, it->second.back()}));
                std::cout << "update text " << s.from << ":" << s.name
                          << " (= " << it->second.back() << " ) -> "
                          << this->entry.name << std::endl;
            }
        }
        break;
    }
    case kind_recv(MessageKind::value):
    case kind_recv(MessageKind::text):
    case MessageKind::entry:
        std::cerr << "Invalid Message Kind " << static_cast<int>(kind)
                  << std::endl;
        break;
    default:
        std::cerr << "Unknown Message Kind " << static_cast<int>(kind)
                  << std::endl;
        break;
    }
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
    if (entry_update) {
        std::cerr << "Update Entry " << this->entry.name << std::endl;
        for (const auto &c : store.clients) {
            c.second->send(pack(this->entry));
        }
    }
}
} // namespace WebCFace::Server

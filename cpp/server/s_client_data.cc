#include "s_client_data.h"
#include "store.h"
#include "../message/message.h"

#include <iostream>

namespace WebCFace::Server {
void ClientData::onRecv(const std::string &message) {
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
    switch (kind) {
    case MessageKind::name:
        name = std::any_cast<Name>(obj).name;
        std::cout << this->name << ": connected" << std::endl;
        store.clients_by_name.emplace(name, store.getClient(con));
        break;
    case MessageKind::call: {
        auto v = std::any_cast<Call>(obj);
        v.caller = this->name;
        std::cout << this->name << ": call [" << v.caller_id << "] "
                  << v.receiver << ":" << v.name << " (args = ";
        for (const auto &a : v.args) {
            std::cout << a << ", ";
        }
        std::cout << ")" << std::endl;
        // そのままターゲットのクライアントに送る
        auto c_it = store.clients_by_name.find(v.receiver);
        if (c_it != store.clients_by_name.end()) {
            auto m = pack(v);
            c_it->second->con->send(&m[0], m.size(),
                                    drogon::WebSocketMessageType::Binary);
        } else {
            // 関数存在しないときの処理
        }
        break;
    }
    case MessageKind::call_response: {
        auto v = std::any_cast<CallResponse>(obj);
        std::cout << this->name << ": call response [" << v.caller_id << "] '"
                  << v.response;
        if (v.is_error) {
            std::cout << "' (error)";
        }
        std::cout << std::endl;
        // そのままcallerに送る
        auto c_it = store.clients_by_name.find(v.caller);
        if (c_it != store.clients_by_name.end()) {
            auto m = pack(v);
            c_it->second->con->send(&m[0], m.size(),
                                    drogon::WebSocketMessageType::Binary);
        }
        break;
    }
    case MessageKind::value: {
        auto v = std::any_cast<Value>(obj);
        std::cout << this->name << ": value " << v.name << " = " << v.data
                  << std::endl;
        value_history[v.name].push_back(v.data);
        // このvalueをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            for (const auto &s : c.second->value_subsc) {
                if (s.first == this->name && s.second == v.name) {
                    auto m = pack(Recv<Value>{{}, this->name, v.name, v.data});
                    c.second->con->send(&m[0], m.size(),
                                        drogon::WebSocketMessageType::Binary);
                    std::cout << "update value " << this->name << ":" << v.name
                              << " (= " << v.data << " ) -> " << c.second->name
                              << std::endl;
                }
            }
        }
        break;
    }
    case MessageKind::text: {
        auto v = std::any_cast<Text>(obj);
        std::cout << this->name << ": text " << v.name << " = " << v.data
                  << std::endl;
        text_history[v.name].push_back(v.data);
        // このvalueをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            for (const auto &s : c.second->text_subsc) {
                if (s.first == this->name && s.second == v.name) {
                    auto m = pack(Recv<Text>{{}, this->name, v.name, v.data});
                    c.second->con->send(&m[0], m.size(),
                                        drogon::WebSocketMessageType::Binary);
                    std::cout << "update text " << this->name << ":" << v.name
                              << " (= " << v.data << " ) -> " << c.second->name
                              << std::endl;
                }
            }
        }
        break;
    }
    case kind_subscribe(MessageKind::value): {
        auto s = std::any_cast<Subscribe<Value>>(obj);
        std::cout << this->name << ": subscribe value " << s.from << ":"
                  << s.name << std::endl;
        value_subsc.insert(std::make_pair(s.from, s.name));
        // 指定した値を返す
        auto c_it = store.clients_by_name.find(s.from);
        if (c_it != store.clients_by_name.end()) {
            auto it = c_it->second->value_history.find(s.name);
            if (it != c_it->second->value_history.end()) {
                auto m =
                    pack(Recv<Value>{{}, s.from, s.name, it->second.back()});
                this->con->send(&m[0], m.size(),
                                drogon::WebSocketMessageType::Binary);
                std::cout << "update value " << s.from << ":" << s.name
                          << " (= " << it->second.back() << " ) -> "
                          << this->name << std::endl;
            }
        }
        break;
    }
    case kind_subscribe(MessageKind::text): {
        auto s = std::any_cast<Subscribe<Text>>(obj);
        std::cout << this->name << ": subscribe text " << s.from << ":"
                  << s.name << std::endl;
        text_subsc.insert(std::make_pair(s.from, s.name));
        // 指定した値を返す
        auto c_it = store.clients_by_name.find(s.from);
        if (c_it != store.clients_by_name.end()) {
            auto it = c_it->second->text_history.find(s.name);
            if (it != c_it->second->text_history.end()) {
                auto m =
                    pack(Recv<Text>{{}, s.from, s.name, it->second.back()});
                this->con->send(&m[0], m.size(),
                                drogon::WebSocketMessageType::Binary);
                std::cout << "update text " << s.from << ":" << s.name
                          << " (= " << it->second.back() << " ) -> "
                          << this->name << std::endl;
            }
        }
        break;
    }
    }
}
} // namespace WebCFace::Server

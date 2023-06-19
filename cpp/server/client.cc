#include "client.h"
#include "store.h"
#include "../message/message.h"

#include <iostream>

namespace WebCFace::Server {
void Client::onRecv(const std::string &message) {
    using namespace WebCFace::Message;
    auto [kind, obj] = unpack(message);
    switch (kind) {
    case MessageKind::name:
        name = std::any_cast<Name>(obj).name;
        std::cout << this->name << ": connected" << std::endl;
        store.clients_by_name.emplace(name, store.getClient(con));
        break;
    case MessageKind::value: {
        auto v = std::any_cast<Value>(obj);
        std::cout << this->name << ": value " << v.name << " = " << v.data
                  << std::endl;
        value_history[v.name].push_back(v.data);
        // このvalueをsubscribeしてるところに送り返す
        for (const auto &c : store.clients) {
            for (const auto &s : c.second->value_subsc) {
                if (s.first == this->name && s.second == v.name) {
                    c.second->con->send(
                        pack(Recv<Value>{{}, this->name, v.name, v.data}));
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
                    c.second->con->send(
                        pack(Recv<Text>{{}, this->name, v.name, v.data}));
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
                this->con->send(
                    pack(Recv<Value>{{}, s.from, s.name, it->second.back()}));
                std::cout << "update value " << s.from << ":" << s.name
                          << " (= " << it->second.back() << " ) -> " << this->name
                          << std::endl;
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
                this->con->send(
                    pack(Recv<Text>{{}, s.from, s.name, it->second.back()}));
                std::cout << "update text " << s.from << ":" << s.name
                          << " (= " << it->second.back() << " ) -> " << this->name
                          << std::endl;
            }
        }
        break;
    }
    }
}
} // namespace WebCFace::Server

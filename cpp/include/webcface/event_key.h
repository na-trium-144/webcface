#pragma once
#include <string>
#include <cassert>
#include <eventpp/eventqueue.h>

namespace WebCFace {
enum class EventType {
    none,
    member_entry,
    value_entry,
    text_entry,
    func_entry,
    value_change,
    text_change,
};
//! Eventの種類を表すキー、かつコールバックに返す引数
//! Eventの各種情報と相互キャストできるようにすること。
struct EventKey {
    EventType type = EventType::none;
    std::string member = "", name = "";

    EventKey() = default;
    EventKey(EventType type, const std::string &member = "",
             const std::string &name = "")
        : type(type), member(member), name(name) {}

    bool operator==(const EventKey &rhs) const {
        if (type != rhs.type) {
            return false;
        }
        switch (type) {
        case EventType::member_entry:
            // memberはイベントの内容
            return true;
        case EventType::value_entry:
        case EventType::text_entry:
        case EventType::func_entry:
            // memberはキー、nameは内容
            return member == rhs.member;
        case EventType::value_change:
        case EventType::text_change:
            // member, nameがキー
            return member == rhs.member && name == rhs.name;
        default:
            assert(!"unknown event");
        }
    }
    bool operator!=(const EventKey &rhs) const { return !(*this == rhs); }
    bool operator<(const EventKey &rhs) const {
        if (type != rhs.type) {
            return static_cast<int>(type) < static_cast<int>(rhs.type);
        }
        switch (type) {
        case EventType::member_entry:
            // memberはイベントの内容
            return false;
        case EventType::value_entry:
        case EventType::text_entry:
        case EventType::func_entry:
            // memberはキー、nameは内容
            return member < rhs.member;
        case EventType::value_change:
        case EventType::text_change:
            // member, nameがキー
            return member < rhs.member ||
                   (member == rhs.member && name < rhs.name);
        default:
            assert(!"unknown event");
        }
    }
};

class ClientData;
struct EventPolicies {
    static EventKey getEvent(const EventKey &key, const std::shared_ptr<ClientData> &) {
        return key;
    }
};
using EventQueue = eventpp::EventQueue<
    EventKey, void(const EventKey &, const std::shared_ptr<ClientData> &),
    EventPolicies>;

} // namespace WebCFace
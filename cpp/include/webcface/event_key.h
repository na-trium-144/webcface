#pragma once
#include <cassert>
#include "field_base.h"

namespace WebCFace {

enum class EventType {
    none,
    member_entry,
    value_entry,
    text_entry,
    func_entry,
    value_change,
    text_change,
    log_change,
};

//! Eventの種類を表すキー、かつEventのコールバックに返す引数
struct EventKey : FieldBase {
    EventType type = EventType::none;

    EventKey() = default;
    //! type=member_entryの場合
    //! memberは設定しない
    EventKey(EventType type, const std::weak_ptr<ClientData> &data_w)
        : FieldBase(data_w, ""), type(type) {
        assert(type == EventType::member_entry);
    }
    EventKey(EventType type, const FieldBase &base)
        : FieldBase(base), type(type) {}

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
        case EventType::log_change:
            // memberはキー、nameは内容
            return member_ == rhs.member_;
        case EventType::value_change:
        case EventType::text_change:
            // member, nameがキー
            return member_ == rhs.member_ && field_ == rhs.field_;
        default:
            assert(!"unknown event");
        }
        return false;
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
        case EventType::log_change:
            // memberはキー、nameは内容
            return member_ < rhs.member_;
        case EventType::value_change:
        case EventType::text_change:
            // member, nameがキー
            return member_ < rhs.member_ ||
                   (member_ == rhs.member_ && field_ < rhs.field_);
        default:
            assert(!"unknown event");
        }
        return false;
    }
};

} // namespace WebCFace
#pragma once
#include <string>
#include <cassert>
#include <eventpp/eventqueue.h>
#include "decl.h"

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


//! eventpp::EventQueueのラッパー
/*! イベントはコンストラクタで固定、callbackを渡すだけで登録できるようにする
 * V = コールバックの引数の型
 */
template <typename V>
class EventTarget {
  public:
    using EventQueue = eventpp::EventQueue<EventKey, void(const EventKey &)>;
    using EventHandle = EventQueue::Handle;
    using EventCallback = std::function<void(V)>;

  private:
    std::shared_ptr<ClientData> data;
    EventKey key;
    std::function<void()> on_append = nullptr;

  protected:
    void triggerEvent() { data->event_queue.enqueue(key); }

  public:
    EventTarget() = default;
    explicit EventTarget(const std::shared_ptr<ClientData> &data, EventType type,
                         const std::string &member = "",
                         const std::string &name = "",
                         std::function<void()> on_append = nullptr)
        : queue(queue), key(type, cli, member, name), on_append(on_append) {}

    EventHandle appendListener(const EventCallback &callback) const {
        if (on_append) {
            on_append();
        }
        return data->event_queue.appendListener(key, callback);
    }
    EventHandle prependListener(const EventCallback &callback) const {
        if (on_append) {
            on_append();
        }
        return data->event_queue.prependListener(key, callback);
    }
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const {
        if (on_append) {
            on_append();
        }
        return data->event_queue.insertListener(key, callback, before);
    }
    bool removeListener(const EventHandle &handle) const {
        return data->event_queue.removeListener(key, handle);
    }
    bool hasAnyListener() const { return data->event_queue.hasAnyListener(key); }
    bool ownsHandle(const EventHandle &handle) const {
        return data->event_queue.ownsHandle(key, handle);
    }
};
} // namespace WebCFace
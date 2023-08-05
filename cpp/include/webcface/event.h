#pragma once
#include <string>
#include <cassert>

namespace WebCFace {

//! eventpp::EventQueueのラッパー
/*! イベントはコンストラクタで固定、callbackを渡すだけで登録できるようにする
 * V = コールバックの引数の型
 */
template <typename V>
class EventTarget {
  public:
    using EventHandle = EventQueue::Handle;
    using EventCallback = std::function<void(V)>;

  private:
    std::weak_ptr<ClientData> data;
    EventKey key;
    std::function<void()> on_append = nullptr;

  protected:
    void triggerEvent() {
        if (auto data_s = data.lock()) {
            data_s->event_queue.enqueue(key);
        }
    }

  public:
    EventTarget() = default;
    explicit EventTarget(const std::weak_ptr<ClientData> &data, EventType type,
                         const std::string &member = "",
                         const std::string &name = "",
                         std::function<void()> on_append = nullptr)
        : data(data), key(data, type, member, name), on_append(on_append) {}

    EventHandle appendListener(const EventCallback &callback) const {
        if (auto data_s = data.lock()) {
            if (on_append) {
                on_append();
            }
            return data_s->event_queue.appendListener(key, callback);
        }
        return EventHandle{};
    }
    EventHandle prependListener(const EventCallback &callback) const {
        if (auto data_s = data.lock()) {
            if (on_append) {
                on_append();
            }
            return data_s->event_queue.prependListener(key, callback);
        }
        return EventHandle{};
    }
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const {
        if (auto data_s = data.lock()) {
            if (on_append) {
                on_append();
            }
            return data_s->event_queue.insertListener(key, callback, before);
        }
        return EventHandle{};
    }
    bool removeListener(const EventHandle &handle) const {
        if (auto data_s = data.lock()) {
            return data_s->event_queue.removeListener(key, handle);
        }
        return false;
    }
    bool hasAnyListener() const {
        if (auto data_s = data.lock()) {
            return data_s->event_queue.hasAnyListener(key);
        }
        return false;
    }
    bool ownsHandle(const EventHandle &handle) const {
        if (auto data_s = data.lock()) {
            return data_s->event_queue.ownsHandle(key, handle);
        }
        return false;
    }
};
} // namespace WebCFace
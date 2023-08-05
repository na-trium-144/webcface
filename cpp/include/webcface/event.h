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
    //! イベントを発生させる。
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

    //! イベントのコールバックをリストの最後に追加する。
    EventHandle appendListener(const EventCallback &callback) const {
        if (auto data_s = data.lock()) {
            if (on_append) {
                on_append();
            }
            return data_s->event_queue.appendListener(key, callback);
        }
        return EventHandle{};
    }
    //! イベントのコールバックをリストの最初に追加する。
    EventHandle prependListener(const EventCallback &callback) const {
        if (auto data_s = data.lock()) {
            if (on_append) {
                on_append();
            }
            return data_s->event_queue.prependListener(key, callback);
        }
        return EventHandle{};
    }
    //! イベントのコールバックを間に挿入する。
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
    //! コールバックを削除する。
    bool removeListener(const EventHandle &handle) const {
        if (auto data_s = data.lock()) {
            return data_s->event_queue.removeListener(key, handle);
        }
        return false;
    }
    //! コールバックが登録されているかを調べる。
    bool hasAnyListener() const {
        if (auto data_s = data.lock()) {
            return data_s->event_queue.hasAnyListener(key);
        }
        return false;
    }
    //! handleがこのイベントのものかを調べる。
    bool ownsHandle(const EventHandle &handle) const {
        if (auto data_s = data.lock()) {
            return data_s->event_queue.ownsHandle(key, handle);
        }
        return false;
    }
};
} // namespace WebCFace
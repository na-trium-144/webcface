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
    std::shared_ptr<ClientData> data;
    EventKey key;
    std::function<void()> on_append = nullptr;

  protected:
    void triggerEvent() { data->event_queue.enqueue(key, data); }
    //! callbackの引数にClientDataを追加する
    //! listenerはClientDataに保存されるので循環参照を防ぐため含まない
    //! callbackの中身がClientData含んだらどうする?
    auto wrapCallback(const EventCallback &callback) {
        return [callback](const EventKey &key,
                          const std::shared_ptr<ClientData> &data) {
            callback(V{key, data});
        };
    }

  public:
    EventTarget() = default;
    explicit EventTarget(const std::shared_ptr<ClientData> &data,
                         EventType type, const std::string &member = "",
                         const std::string &name = "",
                         std::function<void()> on_append = nullptr)
        : data(data), key(type, member, name), on_append(on_append) {}

    EventHandle appendListener(const EventCallback &callback) const {
        if (on_append) {
            on_append();
        }
        return data->event_queue.appendListener(key, wrapCallback(callback));
    }
    EventHandle prependListener(const EventCallback &callback) const {
        if (on_append) {
            on_append();
        }
        return data->event_queue.prependListener(key, wrapCallback(callback));
    }
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const {
        if (on_append) {
            on_append();
        }
        return data->event_queue.insertListener(key, wrapCallback(callback),
                                                before);
    }
    bool removeListener(const EventHandle &handle) const {
        return data->event_queue.removeListener(key, handle);
    }
    bool hasAnyListener() const {
        return data->event_queue.hasAnyListener(key);
    }
    bool ownsHandle(const EventHandle &handle) const {
        return data->event_queue.ownsHandle(key, handle);
    }
};
} // namespace WebCFace
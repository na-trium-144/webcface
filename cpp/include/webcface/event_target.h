#pragma once
#include <functional>
#include "event_key.h"
#include "client_data.h"

namespace WebCFace {

using EventHandle = ClientData::EventQueue::Handle;

//! eventpp::EventQueueのラッパー
/*! イベントはコンストラクタで固定、callbackを渡すだけで登録できるようにする
 * V = コールバックの引数の型 (FieldBaseから変換可能である必要がある)
 */
template <typename V>
class EventTarget {
    //! listenerを追加する際に行わなければならない処理があれば登録する
    std::function<void()> on_append = nullptr;
    EventKey key;
    using EventCallback = std::function<void(V)>;

  protected:
    //! イベントを発生させる。
    void triggerEvent() const { key.dataLock()->event_queue.enqueue(key); }

  public:
    EventTarget() = default;
    EventTarget(EventType type, const std::weak_ptr<ClientData> &data_w,
                std::function<void()> on_append = nullptr)
        : key(type, data_w), on_append(on_append) {}
    EventTarget(EventType type, const Field &base,
                std::function<void()> on_append = nullptr)
        : key(type, base), on_append(on_append) {}

    //! イベントのコールバックをリストの最後に追加する。
    EventHandle appendListener(const EventCallback &callback) const {
        if (on_append) {
            on_append();
        }
        return key.dataLock()->event_queue.appendListener(key, callback);
    }
    //! イベントのコールバックをリストの最初に追加する。
    EventHandle prependListener(const EventCallback &callback) const {
        if (on_append) {
            on_append();
        }
        return key.dataLock()->event_queue.prependListener(key, callback);
    }
    //! イベントのコールバックを間に挿入する。
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const {
        if (on_append) {
            on_append();
        }
        return key.dataLock()->event_queue.insertListener(key, callback, before);
    }
    //! コールバックを削除する。
    bool removeListener(const EventHandle &handle) const {
        return key.dataLock()->event_queue.removeListener(key, handle);
    }
    //! コールバックが登録されているかを調べる。
    bool hasAnyListener() const {
        return key.dataLock()->event_queue.hasAnyListener(key);
    }
    //! handleがこのイベントのものかを調べる。
    bool ownsHandle(const EventHandle &handle) const {
        return key.dataLock()->event_queue.ownsHandle(key, handle);
    }
};
} // namespace WebCFace
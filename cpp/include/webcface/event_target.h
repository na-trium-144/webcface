#pragma once
#include <functional>
#include "client_data.h"

namespace WebCFace {

//! eventpp::EventQueueのラッパー
/*! イベントはコンストラクタで固定、callbackを渡すだけで登録できるようにする
 * V = コールバックの引数の型
 */
template <typename V, typename Dispatcher>
class EventTarget {
    using EventCallback = std::function<void(V)>;
    using EventHandle = typename Dispatcher::Handle;
    using EventKey = typename Dispatcher::Event;

    Dispatcher *dispatcher;
    EventKey key;

  protected:
    //! イベントを発生させる。
    void triggerEvent(const V &arg) const { dispatcher->dispatch(key, arg); }

    //! listenerを追加する際に行わなければならない処理があればoverrideする
    // virtual void onAppend() const {}

  public:
    EventTarget() = default;
    EventTarget(Dispatcher *dispatcher, const EventKey &key)
        : dispatcher(dispatcher), key(key) {}

    //! イベントのコールバックをリストの最後に追加する。
    EventHandle appendListener(const EventCallback &callback) const {
        // onAppend();
        return dispatcher->appendListener(key, callback);
    }
    //! イベントのコールバックをリストの最初に追加する。
    EventHandle prependListener(const EventCallback &callback) const {
        // onAppend();
        return dispatcher->prependListener(key, callback);
    }
    //! イベントのコールバックを間に挿入する。
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const {
        // onAppend();
        return dispatcher->insertListener(key, callback, before);
    }
    //! コールバックを削除する。
    bool removeListener(const EventHandle &handle) const {
        return dispatcher->removeListener(key, handle);
    }
    //! コールバックが登録されているかを調べる。
    bool hasAnyListener() const { return dispatcher->hasAnyListener(key); }
    //! handleがこのイベントのものかを調べる。
    bool ownsHandle(const EventHandle &handle) const {
        return dispatcher->ownsHandle(key, handle);
    }
};
} // namespace WebCFace
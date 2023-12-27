#pragma once
#include <functional>
#include <eventpp/eventdispatcher.h>
#include "field.h"
#include "common/def.h"

namespace WEBCFACE_NS {

/*!
 * \brief イベントを表し、コールバックの追加や削除ができるクラス。
 *
 */
template <typename V, typename Key = FieldBaseComparable,
          typename VBase = Field>
class EventTarget {
    using Dispatcher = eventpp::EventDispatcher<Key, void(VBase)>;
    Dispatcher *dispatcher = nullptr;
    Key key;

  public:
    /*!
     * \brief イベントのコールバックの型
     *
     */
    using EventCallback = std::function<void(V)>;
    /*!
     * \brief コールバックのHandle
     *
     */
    using EventHandle = typename Dispatcher::Handle;

  protected:
    /*!
     * \brief イベントを発生させる。
     *
     */
    void triggerEvent(const VBase &arg) const {
        dispatcher->dispatch(key, arg);
    }

    /*!
     * \brief listenerを追加する際に行わなければならない処理があればoverrideする
     *
     */
    virtual void onAppend() const {}

  public:
    EventTarget() = default;
    EventTarget(Dispatcher *dispatcher, const Key &key)
        : dispatcher(dispatcher), key(key) {}

    virtual ~EventTarget() {}
    
    /*!
     * \brief イベントのコールバックをリストの最後に追加する。
     *
     */
    EventHandle appendListener(const EventCallback &callback) const {
        onAppend();
        return dispatcher->appendListener(key, callback);
    }
    /*!
     * \brief イベントのコールバックをリストの最初に追加する。
     *
     */
    EventHandle prependListener(const EventCallback &callback) const {
        onAppend();
        return dispatcher->prependListener(key, callback);
    }
    /*!
     * \brief イベントのコールバックを間に挿入する。
     *
     */
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const {
        onAppend();
        return dispatcher->insertListener(key, callback, before);
    }
    /*!
     * \brief コールバックを削除する。
     *
     */
    bool removeListener(const EventHandle &handle) const {
        return dispatcher->removeListener(key, handle);
    }
    /*!
     * \brief コールバックが登録されているかを調べる。
     *
     */
    bool hasAnyListener() const { return dispatcher->hasAnyListener(key); }
    /*!
     * \brief handleがこのイベントのものかを調べる。
     *
     */
    bool ownsHandle(const EventHandle &handle) const {
        return dispatcher->ownsHandle(key, handle);
    }
};
} // namespace WEBCFACE_NS

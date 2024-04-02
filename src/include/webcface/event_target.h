#pragma once
#include <functional>
#include <eventpp/eventdispatcher.h>
#include "field.h"
#include "common/def.h"

WEBCFACE_NS_BEGIN

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
     * \brief イベントのコールバックをリストの最後に追加する。
     * (引数なしのコールバック)
     * \since ver1.7
     *
     */
    template <typename F>
        requires std::invocable<F>
    EventHandle appendListener(const F &callback) const {
        return appendListener([callback](const auto &) { callback(); });
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
     * \brief イベントのコールバックをリストの最初に追加する。
     * (引数なしのコールバック)
     * \since ver1.7
     *
     */
    template <typename F>
        requires std::invocable<F>
    EventHandle prependListener(const F &callback) const {
        return prependListener([callback](const auto &) { callback(); });
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
     * \brief イベントのコールバックを間に挿入する。
     * (引数なしのコールバック)
     * \since 1.7
     *
     */
    template <typename F>
        requires std::invocable<F>
    EventHandle insertListener(const F &callback,
                               const EventHandle &before) const {
        return insertListener([callback](const auto &) { callback(); }, before);
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

WEBCFACE_NS_END

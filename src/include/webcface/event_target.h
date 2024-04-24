#pragma once
#include <functional>
#include <stdexcept>
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
  public:
    using CallbackList = eventpp::CallbackList<void(VBase)>;

  protected:
    CallbackList *cl = nullptr;

    CallbackList &checkCl() const {
        if (!cl) {
            throw std::runtime_error("CallbackList is null");
        }
        return *cl;
    }

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
    using EventHandle = typename CallbackList::Handle;

  protected:
    /*!
     * \brief イベントを発生させる。
     *
     */
    void triggerEvent(const VBase &arg) const { checkCl()(arg); }

    /*!
     * \brief listenerを追加する際に行わなければならない処理があればoverrideする
     *
     */
    virtual void onAppend() const {}

  public:
    EventTarget() = default;
    explicit EventTarget(CallbackList *cl) : cl(cl) {}

    virtual ~EventTarget() {}

    /*!
     * \brief CallbackListを参照する。
     * \since ver1.11
     *
     * eventppのユーティリティ関数を利用する場合に使える。
     *
     * callbackListにappendListerされるかどうかを判断できないため、
     * onAppend()はこの時点で呼び出される。
     *
     */
    CallbackList &callbackList() const {
        onAppend();
        return checkCl();
    }

    /*!
     * \brief イベントのコールバックをリストの最後に追加する。
     *
     */
    EventHandle appendListener(const EventCallback &callback) const {
        onAppend();
        return checkCl().appendListener(callback);
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
        return checkCl().prependListener(callback);
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
        return checkCl().insertListener(callback, before);
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
        return checkCl().removeListener(handle);
    }
    /*!
     * \brief コールバックが登録されているかを調べる。
     *
     */
    bool hasAnyListener() const { return checkCl().hasAnyListener(); }
    /*!
     * \brief handleがこのイベントのものかを調べる。
     *
     */
    bool ownsHandle(const EventHandle &handle) const {
        return checkCl().ownsHandle(handle);
    }
};

WEBCFACE_NS_END

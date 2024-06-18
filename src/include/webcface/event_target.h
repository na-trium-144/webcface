#pragma once
#include <functional>
#include <stdexcept>
#include <eventpp/callbacklist.h>
#include "field.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN

/*!
 * \brief イベントを表し、コールバックの追加や削除ができるクラス。
 *
 */
template <typename ArgType>
class WEBCFACE_DLL_TEMPLATE EventTarget {
  public:
    using CallbackList = eventpp::CallbackList<void(ArgType)>;

  protected:
    std::weak_ptr<CallbackList> cl;

    CallbackList &checkCl() const {
        if (cl.expired()) {
            throw std::runtime_error("CallbackList is null");
        }
        return *cl.lock();
    }

  public:
    /*!
     * \brief イベントのコールバックの型
     *
     */
    using EventCallback = std::function<void(ArgType)>;
    /*!
     * \brief コールバックのHandle
     *
     */
    using EventHandle = typename CallbackList::Handle;

  protected:
    /*!
     * \brief 参照するコールバックリストを設定して初期化
     *
     * コールバックリストのポインタを参照で渡し、
     * ポインタが未初期化の場合ここで初期化する
     *
     */
    void setCL(std::shared_ptr<CallbackList> &cl) {
        if (!cl) {
            cl = std::make_shared<CallbackList>();
        }
        this->cl = cl;
    }

    /*!
     * \brief イベントを発生させる。
     *
     */
    void triggerEvent(const ArgType &arg) const { checkCl()(arg); }

    /*!
     * \brief listenerを追加する際に行わなければならない処理があればoverrideする
     *
     */
    virtual void onAppend() const {}

  public:
    EventTarget() = default;
    /*!
     * \brief 参照するコールバックリストを設定して初期化
     *
     * コールバックリストのポインタを参照で渡し、
     * ポインタが未初期化の場合ここで初期化する
     *
     */
    explicit EventTarget(std::shared_ptr<CallbackList> &cl) { setCL(cl); }

    virtual ~EventTarget();

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
    CallbackList &callbackList() const;

    /*!
     * \brief イベントのコールバックをリストの最後に追加する。
     *
     */
    EventHandle appendListener(const EventCallback &callback) const;
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
    EventHandle prependListener(const EventCallback &callback) const;
    /*!
     * \brief イベントのコールバックをリストの最初に追加する。
     * (引数なしのコールバック)
     * \since ver1.7
     *
     */
    template <typename F>
        requires std::invocable<F>
    EventHandle prependListener(const F &callback) const;
    /*!
     * \brief イベントのコールバックを間に挿入する。
     *
     */
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const;
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
    bool removeListener(const EventHandle &handle) const;
    /*!
     * \brief コールバックが登録されているかを調べる。
     *
     */
    bool hasAnyListener() const;
    /*!
     * \brief handleがこのイベントのものかを調べる。
     *
     */
    bool ownsHandle(const EventHandle &handle) const;
};

class Member;
class Value;
class Text;
class View;
class Image;
class Func;
class RobotModel;
class Canvas2D;
class Canvas3D;
class Log;
#ifdef _WIN32
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Member>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Value>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Text>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<View>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Image>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Func>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<RobotModel>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Canvas2D>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Canvas3D>;
extern template class WEBCFACE_DLL_INSTANCE_DECL EventTarget<Log>;
#endif

WEBCFACE_NS_END

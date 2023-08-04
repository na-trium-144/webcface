#pragma once
#include <string>
#include <eventpp/callbacklist.h>
#include "decl.h"
#include "func.h"
#include "data_event.h"

namespace WebCFace {

//! Entryが更新された時呼び出すcallbacklistのラッパー
/*! CallbackList をそのまま公開すると
 * SyncDataWithEvent で使っているEventDispacherとインタフェースが微妙に違うので
 * EventDispacher のほうに合わせて関数名を変えてある
 */
template <typename V>
class MemberEvent {
  public:
    using CallbackList = eventpp::CallbackList<void(V)>;
    using EventCallback = CallbackList::Callback;
    using EventHandle = CallbackList::Handle;

  private:
    CallbackList *callback_list;

  public:
    explicit MemberEvent(CallbackList *callback_list)
        : callback_list(callback_list) {}

    EventHandle appendListener(const EventCallback &callback) const {
        return callback_list->append(callback);
    }
    EventHandle prependListener(const EventCallback &callback) const {
        return callback_list->prepend(callback);
    }
    EventHandle insertListener(const EventCallback &callback,
                               const EventHandle &before) const {
        return callback_list->insert(callback, before);
    }
    bool removeListener(const EventHandle &handle) const {
        return callback_list->remove(handle);
    }
    bool hasAnyListener() const { return !callback_list->empty(); }
    bool ownsHandle(const EventHandle &handle) const {
        return callback_list->ownsHandle(handle);
    }
};

//! 他のクライアントを参照することを表すクラス
/*! 参照元のClientが破棄されているとセグフォします
 */
class Member {
    Client *cli = nullptr;
    std::string name_;

  public:
    Member() = default;
    Member(Client *cli, const std::string &name);

    std::string name() const { return name_; }

    //! このmemberの指定した名前のvalueを参照する。
    Value value(const std::string &name) const {
        return Value{cli, this->name(), name};
    }
    //! このmemberの指定した名前のtextを参照する。
    Text text(const std::string &name) const {
        return Text{cli, this->name(), name};
    }
    //! このmemberの指定した名前のfuncを参照する。
    Func func(const std::string &name) const {
        return Func{cli, this->name(), name};
    }
    //! このmemberが公開しているvalueのリストを返す。
    std::vector<Value> values() const;
    //! このmemberが公開しているtextのリストを返す。
    std::vector<Text> texts() const;
    //! このmemberが公開しているfuncのリストを返す。
    std::vector<Func> funcs() const;

    //! valueが追加された時のイベントリスト
    MemberEvent<Value> valuesChange();
    //! textが追加された時のイベントリスト
    MemberEvent<Text> textsChange();
    //! funcが追加された時のイベントリスト
    MemberEvent<Func> funcsChange();
};

} // namespace WebCFace

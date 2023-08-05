#pragma once
#include <string>
#include <eventpp/callbacklist.h>
#include "decl.h"
#include "func.h"
#include "event.h"

namespace WebCFace {

//! 他のクライアントを参照することを表すクラス
/*! 参照元のClientが破棄されているとセグフォします
 */
class Member {
    std::shared_ptr<ClientData> data;
    std::string name_;

  public:
    Member() = default;
    Member(const std::shared_ptr<ClientData> &data, const std::string &name);
    Member(const EventKey &key) : Member(key.data, key.member) {}

    std::string name() const { return name_; }

    //! このmemberの指定した名前のvalueを参照する。
    Value value(const std::string &name) const {
        return Value{data, this->name(), name};
    }
    //! このmemberの指定した名前のtextを参照する。
    Text text(const std::string &name) const {
        return Text{data, this->name(), name};
    }
    //! このmemberの指定した名前のfuncを参照する。
    Func func(const std::string &name) const {
        return Func{data, this->name(), name};
    }
    //! このmemberが公開しているvalueのリストを返す。
    std::vector<Value> values() const;
    //! このmemberが公開しているtextのリストを返す。
    std::vector<Text> texts() const;
    //! このmemberが公開しているfuncのリストを返す。
    std::vector<Func> funcs() const;

    //! valueが追加された時のイベントリスト
    EventTarget<Value> valuesChange() const;
    //! textが追加された時のイベントリスト
    EventTarget<Text> textsChange() const;
    //! funcが追加された時のイベントリスト
    EventTarget<Func> funcsChange() const;
};

} // namespace WebCFace

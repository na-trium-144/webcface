#pragma once
#include <string>
#include <eventpp/callbacklist.h>

namespace WebCFace {

template <typename V>
class EventTarget;
class Value;
class Text;
class Func;
class ClientData;

//! 他のクライアントを参照することを表すクラス
class Member {
    std::weak_ptr<ClientData> data;
    std::string name_;

  public:
    Member() = default;
    Member(const std::weak_ptr<ClientData> &data, const std::string &name)
        : data(data), name_(name) {}
    Member(const EventKey &key) : Member(key.data, key.member) {}

    std::string name() const { return name_; }

    //! このmemberの指定した名前のvalueを参照する。
    Value value(const std::string &name) const;
    //! このmemberの指定した名前のtextを参照する。
    Text text(const std::string &name) const;
    //! このmemberの指定した名前のfuncを参照する。
    Func func(const std::string &name) const;

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

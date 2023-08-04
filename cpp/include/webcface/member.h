#pragma once
#include <string>
#include "decl.h"
#include "func.h"
#include "data_event.h"

namespace WebCFace {

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
};

} // namespace WebCFace

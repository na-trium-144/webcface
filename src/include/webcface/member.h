#pragma once
#include <string>
#include <vector>
#include <optional>
#include "field.h"
#include "event_target.h"

namespace WebCFace {

class Value;
class Text;
class Func;
class Log;
class View;
class ClientData;

//! 他のクライアントを参照することを表すクラス
class Member : protected Field {
  public:
    Member() = default;
    Member(const std::weak_ptr<ClientData> &data_w, const std::string &member)
        : Field(data_w, member) {}
    Member(const Field &base) : Field(base) {}

    //! Member名
    std::string name() const { return member_; }

    //! このmemberの指定した名前のvalueを参照する。
    Value value(const std::string &field) const;
    //! このmemberの指定した名前のtextを参照する。
    Text text(const std::string &field) const;
    //! このmemberの指定した名前のfuncを参照する。
    Func func(const std::string &field) const;
    //! AnonymousFuncオブジェクトを作成しfuncをsetする
    // template <typename T>
    // AnonymousFunc func(const T &func) const{
    // todo:
    // ここでfunc.hにアクセスする必要があるためヘッダーの読み込み順を変えないといけない
    // }
    View view(const std::string &field) const;
    //! このmemberのログを参照する。
    Log log() const;

    //! このmemberが公開しているvalueのリストを返す。
    std::vector<Value> values() const;
    //! このmemberが公開しているtextのリストを返す。
    std::vector<Text> texts() const;
    //! このmemberが公開しているfuncのリストを返す。
    std::vector<Func> funcs() const;
    std::vector<View> views() const;

    //! valueが追加された時のイベントを設定
    /*! このクライアントが接続する前から存在したメンバーについては
     * 初回の sync() 後に一度に送られるので、
     * eventの設定は初回のsync()より前に行うと良い
     * \sa Client::membersChange()
     */
    EventTarget<Value, std::string> onValueEntry() const;
    //! textが追加された時のイベントリスト
    //! \sa valuesChange()
    EventTarget<Text, std::string> onTextEntry() const;
    //! funcが追加された時のイベントリスト
    //! \sa valuesChange()
    EventTarget<Func, std::string> onFuncEntry() const;

    EventTarget<View, std::string> onViewEntry() const;
    EventTarget<Member, std::string> onSync() const;

    std::string libName() const;
    std::string libVersion() const;
    std::string remoteAddr() const;

    std::optional<int> pingStatus() const;
    EventTarget<Member, std::string> onPing() const;
};

} // namespace WebCFace

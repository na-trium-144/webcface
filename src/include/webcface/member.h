#pragma once
#include <string>
#include <vector>
#include <optional>
#include "field.h"
#include "event_target.h"
#include "common/def.h"
#include "value.h"
#include "text.h"
#include "log.h"
#include "func.h"
#include "view.h"

namespace webcface {

namespace Internal {
struct ClientData;
}

//! Memberを指すクラス
/*!
 * コンストラクタではなく Client::member(), Client::members()
 * Client::onMemberEntry() などから取得すること
 */
class WEBCFACE_DLL Member : protected Field {
  public:
    Member() = default;
    Member(const std::weak_ptr<Internal::ClientData> &data_w, const std::string &member)
        : Field(data_w, member) {}
    Member(const Field &base) : Field(base) {}

    //! Member名
    std::string name() const { return member_; }

    //! valueを参照する。
    Value value(const std::string &field) const;
    //! textを参照する。
    Text text(const std::string &field) const;
    //! funcを参照する。
    Func func(const std::string &field) const;

    //! AnonymousFuncオブジェクトを作成しfuncをsetする
    // template <typename T>
    // AnonymousFunc func(const T &func) const{
    // todo:
    // ここでfunc.hにアクセスする必要があるためヘッダーの読み込み順を変えないといけない
    // }

    //! viewを参照する。
    View view(const std::string &field) const;
    //! logを参照する。
    Log log() const;

    //! このmemberが公開しているvalueのリストを返す。
    std::vector<Value> values() const;
    //! このmemberが公開しているtextのリストを返す。
    std::vector<Text> texts() const;
    //! このmemberが公開しているfuncのリストを返す。
    std::vector<Func> funcs() const;
    //! このmemberが公開しているviewのリストを返す。
    std::vector<View> views() const;

    //! valueが追加された時のイベント
    /*! コールバックの型は void(Value)
     */
    EventTarget<Value, std::string> onValueEntry() const;
    //! textが追加された時のイベント
    /*! コールバックの型は void(Text)
     */
    EventTarget<Text, std::string> onTextEntry() const;
    //! funcが追加された時のイベント
    /*! コールバックの型は void(Func)
     */
    EventTarget<Func, std::string> onFuncEntry() const;
    //! viewが追加されたときのイベント
    /*! コールバックの型は void(View)
     */
    EventTarget<View, std::string> onViewEntry() const;
    //! Memberがsync()したときのイベント
    /*! コールバックの型は void(Member)
     */
    EventTarget<Member, std::string> onSync() const;

    //! このMemberが使っているWebCFaceライブラリの識別情報
    /*!
     * このライブラリの場合は"cpp", javascriptクライアントは"js",
     * pythonクライアントは"python"を返す。
     */
    std::string libName() const;
    //! このMemberが使っているWebCFaceのバージョン
    std::string libVersion() const;
    //! このMemberのIPアドレス
    std::string remoteAddr() const;

    //! 通信速度を調べる
    /*!
     * 初回の呼び出しで通信速度データをリクエストし、
     * sync()後通信速度が得られるようになる
     * \return 初回→ std::nullopt, 2回目以降(取得できれば)→ pingの往復時間 (ms)
     * \sa onPing()
     */
    std::optional<int> pingStatus() const;
    //! 通信速度が更新された時のイベント
    /*! コールバックの型は void(Member)
     *
     * 通常は約5秒に1回更新される。
     * pingStatus() と同様、通信速度データのリクエストも行う。
     * \sa pingStatus()
     */
    EventTarget<Member, std::string> onPing() const;
};

} // namespace webcface

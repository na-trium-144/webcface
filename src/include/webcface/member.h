#pragma once
#include <string>
#include <vector>
#include <optional>
#include "field.h"
#include "event_target.h"
#include "common/def.h"

namespace WebCFace {

class Value;
class Text;
class Func;
class Log;
class View;
struct ClientData;

//! Memberを指すクラス
/*!
 * コンストラクタではなく Client::member(), Client::members()
 * Client::onMemberEntry() などから取得すること
 */
class WEBCFACE_DLL Member : protected Field {
  public:
    Member() = default;
    Member(const std::weak_ptr<ClientData> &data_w, const std::string &member)
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
    /*! コールバックを設定する前から存在したデータについてはコールバックは呼び出されない。
     * member名がわかっていれば初回のClient::sync()前に設定するか、
     * Client::onMemberEntry() イベントのコールバックの中で各種イベントを設定すれば間に合う。
     * \return Value追加イベントを指す EventTarget
     */
    EventTarget<Value, std::string> onValueEntry() const;
    //! textが追加された時のイベント
    /*!
     * \sa onValueEntry()
     * \return Text追加イベントを指す EventTarget
     */
    EventTarget<Text, std::string> onTextEntry() const;
    //! funcが追加された時のイベント
    /*!
     * \sa onValueEntry()
     * \return Func追加イベントを指す EventTarget
     */
    EventTarget<Func, std::string> onFuncEntry() const;
    //! viewが追加されたときのイベント
    /*!
     * \sa onValueEntry()
     * \return View追加イベントを指す EventTarget
     */
    EventTarget<View, std::string> onViewEntry() const;
    //! Memberがsync()したときのイベント
    /*!
     * \sa onValueEntry()
     * \return syncイベントを指す EventTarget
     */
    EventTarget<Member, std::string> onSync() const;

    //! このMemberが使っているWebCFaceライブラリの識別情報
    /*!
     * \return このライブラリの場合は"cpp"
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
    /*!
     * 通常は約5秒に1回更新される。
     * 
     * pingStatus() と同様、通信速度データのリクエストも行う。
     * \return pingイベントを指す EventTarget
     * \sa pingStatus()
     */
    EventTarget<Member, std::string> onPing() const;
};

} // namespace WebCFace

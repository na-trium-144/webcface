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
#include "image.h"

namespace webcface {

namespace Internal {
struct ClientData;
}

/*!
 * \brief Memberを指すクラス
 *
 * コンストラクタではなく Client::member(), Client::members()
 * Client::onMemberEntry() などから取得すること
 *
 */
class WEBCFACE_DLL Member : protected Field {
  public:
    Member() = default;
    Member(const std::weak_ptr<Internal::ClientData> &data_w,
           const std::string &member)
        : Field(data_w, member) {}
    Member(const Field &base) : Field(base) {}

    /*!
     * \brief Member名
     *
     */
    std::string name() const { return member_; }

    /*!
     * \brief valueを参照する。
     *
     */
    Value value(const std::string &field) const;
    /*!
     * \brief textを参照する。
     *
     */
    Text text(const std::string &field) const;
    /*!
     * \brief imageを参照する。
     *
     */
    Image image(const std::string &field) const;
    /*!
     * \brief funcを参照する。
     *
     */
    Func func(const std::string &field) const;

    /*!
     * \brief AnonymousFuncオブジェクトを作成しfuncをsetする
     *
     */
    // template <typename T>
    // AnonymousFunc func(const T &func) const{
    // todo:
    // ここでfunc.hにアクセスする必要があるためヘッダーの読み込み順を変えないといけない
    // }

    /*!
     * \brief viewを参照する。
     *
     */
    View view(const std::string &field) const;
    /*!
     * \brief logを参照する。
     *
     */
    Log log() const;

    /*!
     * \brief このmemberが公開しているvalueのリストを返す。
     *
     */
    std::vector<Value> values() const;
    /*!
     * \brief このmemberが公開しているtextのリストを返す。
     *
     */
    std::vector<Text> texts() const;
    /*!
     * \brief このmemberが公開しているfuncのリストを返す。
     *
     */
    std::vector<Func> funcs() const;
    /*!
     * \brief このmemberが公開しているviewのリストを返す。
     *
     */
    std::vector<View> views() const;
    /*!
     * \brief このmemberが公開しているimageのリストを返す。
     *
     */
    std::vector<Image> images() const;

    /*!
     * \brief valueが追加された時のイベント
     *
     * コールバックの型は void(Value)
     *
     */
    EventTarget<Value, std::string> onValueEntry() const;
    /*!
     * \brief textが追加された時のイベント
     *
     * コールバックの型は void(Text)
     *
     */
    EventTarget<Text, std::string> onTextEntry() const;
    /*!
     * \brief funcが追加された時のイベント
     *
     * コールバックの型は void(Func)
     *
     */
    EventTarget<Func, std::string> onFuncEntry() const;
    /*!
     * \brief imageが追加されたときのイベント
     *
     * コールバックの型は void(Image)
     *
     */
    EventTarget<Image, std::string> onImageEntry() const;
    /*!
     * \brief viewが追加されたときのイベント
     *
     * コールバックの型は void(View)
     *
     */
    EventTarget<View, std::string> onViewEntry() const;
    /*!
     * \brief Memberがsync()したときのイベント
     * コールバックの型は void(Member)
     */
    EventTarget<Member, std::string> onSync() const;

    /*!
     * \brief このMemberが使っているWebCFaceライブラリの識別情報
     *
     * \return このライブラリの場合は"cpp", javascriptクライアントは"js",
     * pythonクライアントは"python"を返す。
     *
     */
    std::string libName() const;
    /*!
     * \brief このMemberが使っているWebCFaceのバージョン
     *
     */
    std::string libVersion() const;
    /*!
     * \brief このMemberのIPアドレス
     *
     */
    std::string remoteAddr() const;

    /*!
     * \brief 通信速度を調べる
    /*
     * 初回の呼び出しで通信速度データをリクエストし、
     * sync()後通信速度が得られるようになる
     * \return 初回→ std::nullopt, 2回目以降(取得できれば)→ pingの往復時間 (ms)
     * \sa onPing()
     *
     */
    std::optional<int> pingStatus() const;
    /*!
     * \brief 通信速度が更新された時のイベント
     *
     * コールバックの型は void(Member)
     *
     * 通常は約5秒に1回更新される。
     * pingStatus() と同様、通信速度データのリクエストも行う。
     * \sa pingStatus()
     *
     */
    EventTarget<Member, std::string> onPing() const;
};

} // namespace webcface

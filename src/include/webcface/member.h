#pragma once
#include <string>
#include <vector>
#include <optional>
#include <concepts>
#include "field.h"
#include "event_target.h"
#include "common/def.h"
#include "func.h"

WEBCFACE_NS_BEGIN

namespace Internal {
struct ClientData;
}

class Value;
class Text;
class Log;
class View;
class Image;
class RobotModel;
class Canvas2D;
class Canvas3D;

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

    Value value(const std::string &field) const;
    Text text(const std::string &field) const;
    RobotModel robotModel(const std::string &field) const;
    Image image(const std::string &field) const;
    Func func(const std::string &field) const;
    /*!
     * \brief AnonymousFuncオブジェクトを作成しfuncをsetする
     *
     */
    template <typename T>
        requires(!std::convertible_to<T, std::string>)
    AnonymousFunc func(const T &func) const {
        return AnonymousFunc{*this, func};
    }
    View view(const std::string &field) const;
    Canvas3D canvas3D(const std::string &field) const;
    Canvas2D canvas2D(const std::string &field) const;
    Log log() const;

    /*!
     * \brief このmemberが公開しているvalueのリストを返す。
     *
     */
    std::vector<Value> valueEntries() const;
    /*!
     * \brief このmemberが公開しているvalueのリストを返す。
     * \deprecated 1.6で valueEntries() に変更
     *
     */
    [[deprecated]] std::vector<Value> values() const;
    /*!
     * \brief このmemberが公開しているtextのリストを返す。
     *
     */
    std::vector<Text> textEntries() const;
    /*!
     * \brief このmemberが公開しているtextのリストを返す。
     * \deprecated 1.6で textEntries() に変更
     */
    [[deprecated]] std::vector<Text> texts() const;
    /*!
     * \brief このmemberが公開しているrobotModelのリストを返す。
     *
     */
    std::vector<RobotModel> robotModelEntries() const;
    /*!
     * \brief このmemberが公開しているrobotModelのリストを返す。
     * \deprecated 1.6で robotModelEntries() に変更
     *
     */
    [[deprecated]] std::vector<RobotModel> robotModels() const;
    /*!
     * \brief このmemberが公開しているfuncのリストを返す。
     *
     */
    std::vector<Func> funcEntries() const;
    /*!
     * \brief このmemberが公開しているfuncのリストを返す。
     * \deprecated 1.6で funcEntries() に変更
     *
     */
    [[deprecated]] std::vector<Func> funcs() const;
    /*!
     * \brief このmemberが公開しているviewのリストを返す。
     *
     */
    std::vector<View> viewEntries() const;
    /*!
     * \brief このmemberが公開しているviewのリストを返す。
     * \deprecated 1.6で viewEntries() に変更
     */
    [[deprecated]] std::vector<View> views() const;
    /*!
     * \brief このmemberが公開しているcanvas3dのリストを返す。
     *
     */
    std::vector<Canvas3D> canvas3DEntries() const;
    /*!
     * \brief このmemberが公開しているcanvas2dのリストを返す。
     *
     */
    std::vector<Canvas2D> canvas2DEntries() const;
    /*!
     * \brief このmemberが公開しているimageのリストを返す。
     *
     */
    std::vector<Image> imageEntries() const;
    /*!
     * \brief このmemberが公開しているimageのリストを返す。
     * \deprecated 1.6で imageEntries() に変更
     */
    [[deprecated]] std::vector<Image> images() const;

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
     * \brief robotModelが追加された時のイベント
     *
     * コールバックの型は void(RobotModel)
     *
     */
    EventTarget<RobotModel, std::string> onRobotModelEntry() const;
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
     * \brief canvas3dが追加されたときのイベント
     *
     * コールバックの型は void(Canvas3D)
     *
     */
    EventTarget<Canvas3D, std::string> onCanvas3DEntry() const;
    /*!
     * \brief canvas2dが追加されたときのイベント
     *
     * コールバックの型は void(Canvas2D)
     *
     */
    EventTarget<Canvas2D, std::string> onCanvas2DEntry() const;
    /*!
     * \brief Memberがsync()したときのイベント
     * コールバックの型は void(Member)
     */
    EventTarget<Member, std::string> onSync() const;

    /*!
     * \brief 最後のsync()の時刻を返す
     * \since ver1.7 (Value::time(), Text::time() 等から変更)
     *
     */
    std::chrono::system_clock::time_point syncTime() const;
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
     *
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

WEBCFACE_NS_END

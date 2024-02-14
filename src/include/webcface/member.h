#pragma once
#include <string>
#include <vector>
#include <optional>
#include <concepts>
#include "field.h"
#include "event_target.h"
#include "common/def.h"
#include "value.h"
#include "text.h"
#include "log.h"
#include "func.h"
#include "view.h"
#include "image.h"
#include "robot_model.h"
#include "canvas3d.h"
#include "canvas2d.h"

namespace WEBCFACE_NS {

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
class Member : protected Field {
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

    Value value(const std::string &field) const { return Value{*this, field}; }
    Text text(const std::string &field) const { return Text{*this, field}; }
    RobotModel robotModel(const std::string &field) const {
        return RobotModel{*this, field};
    }
    Image image(const std::string &field) const { return Image{*this, field}; }
    Func func(const std::string &field) const { return Func{*this, field}; }
    /*!
     * \brief AnonymousFuncオブジェクトを作成しfuncをsetする
     *
     */
    template <typename T>
        requires(!std::convertible_to<T, std::string>)
    AnonymousFunc func(const T &func) const {
        return AnonymousFunc{*this, func};
    }
    View view(const std::string &field) const { return View{*this, field}; }
    Canvas3D canvas3D(const std::string &field) const {
        return Canvas3D{*this, field};
    }
    Canvas2D canvas2D(const std::string &field) const {
        return Canvas2D{*this, field};
    }
    Log log() const { return Log{*this}; }

    /*!
     * \brief このmemberが公開しているvalueのリストを返す。
     *
     * (ver1.6で values から名前変更)
     *
     */
    WEBCFACE_DLL std::vector<Value> valueEntries() const;
    [[deprecated]] std::vector<Value> values() const { return valueEntries(); }
    /*!
     * \brief このmemberが公開しているtextのリストを返す。
     *
     * (ver1.6で texts から名前変更)
     *
     */
    WEBCFACE_DLL std::vector<Text> textEntries() const;
    [[deprecated]] std::vector<Text> texts() const { return textEntries(); }
    /*!
     * \brief このmemberが公開しているrobotModelのリストを返す。
     *
     * (ver1.6で robotModels から名前変更)
     *
     */
    WEBCFACE_DLL std::vector<RobotModel> robotModelEntries() const;
    [[deprecated]] std::vector<RobotModel> robotModels() const {
        return robotModelEntries();
    }
    /*!
     * \brief このmemberが公開しているfuncのリストを返す。
     *
     * (ver1.6で funcs から名前変更)
     *
     */
    WEBCFACE_DLL std::vector<Func> funcEntries() const;
    [[deprecated]] std::vector<Func> funcs() const { return funcEntries(); }
    /*!
     * \brief このmemberが公開しているviewのリストを返す。
     *
     * (ver1.6で views から名前変更)
     *
     */
    WEBCFACE_DLL std::vector<View> viewEntries() const;
    [[deprecated]] std::vector<View> views() const { return viewEntries(); }
    /*!
     * \brief このmemberが公開しているcanvas3dのリストを返す。
     *
     */
    WEBCFACE_DLL std::vector<Canvas3D> canvas3DEntries() const;
    /*!
     * \brief このmemberが公開しているcanvas2dのリストを返す。
     *
     */
    WEBCFACE_DLL std::vector<Canvas2D> canvas2DEntries() const;
    /*!
     * \brief このmemberが公開しているimageのリストを返す。
     *
     * (ver1.6で images から名前変更)
     *
     */
    WEBCFACE_DLL std::vector<Image> imageEntries() const;
    [[deprecated]] std::vector<Image> images() const { return imageEntries(); }

    /*!
     * \brief valueが追加された時のイベント
     *
     * コールバックの型は void(Value)
     *
     */
    WEBCFACE_DLL EventTarget<Value, std::string> onValueEntry() const;
    /*!
     * \brief textが追加された時のイベント
     *
     * コールバックの型は void(Text)
     *
     */
    WEBCFACE_DLL EventTarget<Text, std::string> onTextEntry() const;
    /*!
     * \brief robotModelが追加された時のイベント
     *
     * コールバックの型は void(RobotModel)
     *
     */
    WEBCFACE_DLL EventTarget<RobotModel, std::string> onRobotModelEntry() const;
    /*!
     * \brief funcが追加された時のイベント
     *
     * コールバックの型は void(Func)
     *
     */
    WEBCFACE_DLL EventTarget<Func, std::string> onFuncEntry() const;
    /*!
     * \brief imageが追加されたときのイベント
     *
     * コールバックの型は void(Image)
     *
     */
    WEBCFACE_DLL EventTarget<Image, std::string> onImageEntry() const;
    /*!
     * \brief viewが追加されたときのイベント
     *
     * コールバックの型は void(View)
     *
     */
    WEBCFACE_DLL EventTarget<View, std::string> onViewEntry() const;
    /*!
     * \brief canvas3dが追加されたときのイベント
     *
     * コールバックの型は void(Canvas3D)
     *
     */
    WEBCFACE_DLL EventTarget<Canvas3D, std::string> onCanvas3DEntry() const;
    /*!
     * \brief canvas2dが追加されたときのイベント
     *
     * コールバックの型は void(Canvas2D)
     *
     */
    WEBCFACE_DLL EventTarget<Canvas2D, std::string> onCanvas2DEntry() const;
    /*!
     * \brief Memberがsync()したときのイベント
     * コールバックの型は void(Member)
     */
    WEBCFACE_DLL EventTarget<Member, std::string> onSync() const;

    /*!
     * \brief このMemberが使っているWebCFaceライブラリの識別情報
     *
     * \return このライブラリの場合は"cpp", javascriptクライアントは"js",
     * pythonクライアントは"python"を返す。
     *
     */
    WEBCFACE_DLL std::string libName() const;
    /*!
     * \brief このMemberが使っているWebCFaceのバージョン
     *
     */
    WEBCFACE_DLL std::string libVersion() const;
    /*!
     * \brief このMemberのIPアドレス
     *
     */
    WEBCFACE_DLL std::string remoteAddr() const;

    /*!
     * \brief 通信速度を調べる
     *
     * 初回の呼び出しで通信速度データをリクエストし、
     * sync()後通信速度が得られるようになる
     * \return 初回→ std::nullopt, 2回目以降(取得できれば)→ pingの往復時間 (ms)
     * \sa onPing()
     *
     */
    WEBCFACE_DLL std::optional<int> pingStatus() const;
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
    WEBCFACE_DLL EventTarget<Member, std::string> onPing() const;
};

} // namespace WEBCFACE_NS

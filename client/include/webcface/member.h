#pragma once
#include <string>
#include <vector>
#include <optional>
#include "field.h"
#include "webcface/common/def.h"
#include "func.h"
#include "webcface/log.h"

WEBCFACE_NS_BEGIN

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
    Member(const std::weak_ptr<internal::ClientData> &data_w,
           const SharedString &member)
        : Field(data_w, member) {}
    Member(const Field &base) : Field(base.data_w, base.member_) {}

    /*!
     * \brief Member名
     *
     */
    std::string name() const { return member_.decode(); }
    /*!
     * \brief Member名 (wstring)
     * \since ver2.0
     */
    std::wstring nameW() const { return member_.decodeW(); }

    using Field::child;
    using Field::operator[];

    using Field::canvas2D;
    using Field::canvas3D;
    using Field::func;
    using Field::funcListener;
    using Field::image;
    using Field::robotModel;
    using Field::text;
    using Field::value;
    using Field::view;
    Log log() const;
    /*!
     * \brief AnonymousFuncオブジェクトを作成しfuncをsetする
     *
     */
    template <typename T, typename std::enable_if_t<
                              !std::is_convertible_v<T, std::string_view> &&
                                  !std::is_convertible_v<T, std::wstring_view>,
                              std::nullptr_t> = nullptr>
    AnonymousFunc func(const T &func) const {
        return AnonymousFunc{*this, func};
    }

    using Field::canvas2DEntries;
    using Field::canvas3DEntries;
    using Field::funcEntries;
    using Field::imageEntries;
    using Field::robotModelEntries;
    using Field::textEntries;
    using Field::valueEntries;
    using Field::viewEntries;
    /*!
     * \brief このmemberが公開しているvalueのリストを返す。
     * \deprecated 1.6で valueEntries() に変更
     *
     */
    [[deprecated]] std::vector<Value> values() const;
    /*!
     * \brief このmemberが公開しているtextのリストを返す。
     * \deprecated 1.6で textEntries() に変更
     */
    [[deprecated]] std::vector<Text> texts() const;
    /*!
     * \brief このmemberが公開しているrobotModelのリストを返す。
     * \deprecated 1.6で robotModelEntries() に変更
     *
     */
    [[deprecated]] std::vector<RobotModel> robotModels() const;
    /*!
     * \brief このmemberが公開しているfuncのリストを返す。
     * \deprecated 1.6で funcEntries() に変更
     *
     */
    [[deprecated]] std::vector<Func> funcs() const;
    /*!
     * \brief このmemberが公開しているviewのリストを返す。
     * \deprecated 1.6で viewEntries() に変更
     */
    [[deprecated]] std::vector<View> views() const;
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
    Member &onValueEntry(std::function<void WEBCFACE_CALL_FP(Value)> callback);
    /*!
     * \brief textが追加された時のイベント
     *
     * コールバックの型は void(Text)
     *
     */
    Member &onTextEntry(std::function<void WEBCFACE_CALL_FP(Text)> callback);
    /*!
     * \brief robotModelが追加された時のイベント
     *
     * コールバックの型は void(RobotModel)
     *
     */
    Member &
    onRobotModelEntry(std::function<void WEBCFACE_CALL_FP(RobotModel)> callback);
    /*!
     * \brief funcが追加された時のイベント
     *
     * コールバックの型は void(Func)
     *
     */
    Member &onFuncEntry(std::function<void WEBCFACE_CALL_FP(Func)> callback);
    /*!
     * \brief imageが追加されたときのイベント
     *
     * コールバックの型は void(Image)
     *
     */
    Member &onImageEntry(std::function<void WEBCFACE_CALL_FP(Image)> callback);
    /*!
     * \brief viewが追加されたときのイベント
     *
     * コールバックの型は void(View)
     *
     */
    Member &onViewEntry(std::function<void WEBCFACE_CALL_FP(View)> callback);
    /*!
     * \brief canvas3dが追加されたときのイベント
     *
     * コールバックの型は void(Canvas3D)
     *
     */
    Member &
    onCanvas3DEntry(std::function<void WEBCFACE_CALL_FP(Canvas3D)> callback);
    /*!
     * \brief canvas2dが追加されたときのイベント
     *
     * コールバックの型は void(Canvas2D)
     *
     */
    Member &
    onCanvas2DEntry(std::function<void WEBCFACE_CALL_FP(Canvas2D)> callback);
    /*!
     * \brief Memberがsync()したときのイベント
     * コールバックの型は void(Member)
     */
    Member &onSync(std::function<void WEBCFACE_CALL_FP(Member)> callback);

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
     * * 初回の呼び出しで通信速度データをリクエストし、
     * sync()後通信速度が得られるようになる
     * * ver2.0〜 Client自身に対しても使用可能
     * (Client::pingStatus() または member("自分の名前").pingStatus())
     *
     * \return 初回→ std::nullopt, 2回目以降(取得できれば)→ pingの往復時間 (ms)
     * \sa onPing()
     */
    std::optional<int> pingStatus() const;
    /*!
     * \brief 通信速度が更新された時のイベント
     *
     * * コールバックの型は void(Member)
     * * 通常は約5秒に1回更新される。
     * * pingStatus() と同様、通信速度データのリクエストも行う。
     * * ver2.0〜 Client自身に対しても使用可能
     *
     * \sa pingStatus()
     */
    Member &onPing(std::function<void WEBCFACE_CALL_FP(Member)> callback);

    /*!
     * \brief Memberを比較
     * \since ver1.11
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Member>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Member>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};

WEBCFACE_NS_END

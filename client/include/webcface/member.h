#pragma once
#include <functional>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include "field.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

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

    friend struct internal::ClientData;

    /*!
     * \brief Member名
     *
     * ver2.10〜 std::stringの参照から StringView に変更
     *
     */
    StringView name() const { return member_.decodeShare(); }
    /*!
     * \brief Member名 (wstring)
     * \since ver2.0
     *
     * ver2.10〜 std::wstringの参照から WStringView に変更
     *
     */
    WStringView nameW() const { return member_.decodeShareW(); }

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

    /*!
     * \since ver2.4
     */
    template <WEBCFACE_COMPLETE(Log)>
    Log_ log(StringInitializer name) const {
        return this->child(static_cast<SharedString &>(name));
    }
    /*!
     * ver2.4〜: nameを省略した場合 "default" として送信される。
     *
     */
    template <WEBCFACE_COMPLETE(Log)>
    Log_ log() const;

    using Field::canvas2DEntries;
    using Field::canvas3DEntries;
    using Field::children;
    using Field::childrenRecurse;
    using Field::funcEntries;
    using Field::hasChildren;
    using Field::imageEntries;
    using Field::logEntries;
    using Field::robotModelEntries;
    using Field::textEntries;
    using Field::valueEntries;
    using Field::viewEntries;
    /*!
     * \brief このmemberが公開しているvalueのリストを返す。
     * \deprecated 1.6で valueEntries() に変更
     *
     */
    [[deprecated]]
    std::vector<Value> values() const;
    /*!
     * \brief このmemberが公開しているtextのリストを返す。
     * \deprecated 1.6で textEntries() に変更
     */
    [[deprecated]]
    std::vector<Text> texts() const;
    /*!
     * \brief このmemberが公開しているrobotModelのリストを返す。
     * \deprecated 1.6で robotModelEntries() に変更
     *
     */
    [[deprecated]]
    std::vector<RobotModel> robotModels() const;
    /*!
     * \brief このmemberが公開しているfuncのリストを返す。
     * \deprecated 1.6で funcEntries() に変更
     *
     */
    [[deprecated]]
    std::vector<Func> funcs() const;
    /*!
     * \brief このmemberが公開しているviewのリストを返す。
     * \deprecated 1.6で viewEntries() に変更
     */
    [[deprecated]]
    std::vector<View> views() const;
    /*!
     * \brief このmemberが公開しているimageのリストを返す。
     * \deprecated 1.6で imageEntries() に変更
     */
    [[deprecated]]
    std::vector<Image> images() const;

    /*!
     * \brief Memberのデータが存在するかどうかを返す
     * \since ver2.9
     *
     * * connected()
     * とは異なり、切断後もサーバーにデータが残っていてまだデータを受信でき、trueになる場合がある
     *
     */
    bool exists() const;
    /*!
     * \brief Memberがサーバーに接続できているときtrueを返す
     * \since ver2.9
     *
     * * 自身を表すMemberに対する connected() は、Client::connected() と同じ。
     * * 自身のサーバーとの通信が切断された場合、全メンバーについてfalseとなる
     */
    bool connected() const;
    /*!
     * \brief Memberが切断したときに呼び出されるコールバックを設定
     * \since ver2.9
     * \param callback Member型の引数(thisが渡される)を1つ取る関数
     *
     * * 自身のサーバーとの通信が切断された場合も、全メンバーについて呼び出される。
     */
    const Member &
    onDisconnect(std::function<void WEBCFACE_CALL_FP(Member)> callback) const;
    /*!
     * \brief Memberが切断したときに呼び出されるコールバックを設定
     * \since ver2.9
     * \param callback 引数をとらない関数
     *
     * * 自身のサーバーとの通信が切断された場合も、全メンバーについて呼び出される。
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Member &onDisconnect(F callback) const {
        return onDisconnect(
            [callback = std::move(callback)](const auto &) { callback(); });
    }
    /*!
     * \brief Memberがサーバーに接続したときに呼び出されるコールバックを設定
     * \since ver2.9
     * \param callback Member型の引数(thisが渡される)を1つ取る関数
     *
     * * Client::onMemberEntry の直後に呼び出される。
     */
    const Member &
    onConnect(std::function<void WEBCFACE_CALL_FP(Member)> callback) const;
    /*!
     * \brief Memberがサーバーに接続したときに呼び出されるコールバックを設定
     * \since ver2.9
     * \param callback 引数をとらない関数
     *
     * * Client::onMemberEntry の直後に呼び出される。
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Member &onConnect(F callback) const {
        return onConnect(
            [callback = std::move(callback)](const auto &) { callback(); });
    }

    /*!
     * \brief valueが追加された時のイベント
     * \since ver2.0
     * \param callback Value型の引数をとる関数
     *
     */
    const Member &
    onValueEntry(std::function<void WEBCFACE_CALL_FP(Value)> callback) const;
    /*!
     * \brief textが追加された時のイベント
     * \since ver2.0
     * \param callback Text型の引数をとる関数
     *
     */
    const Member &
    onTextEntry(std::function<void WEBCFACE_CALL_FP(Text)> callback) const;
    /*!
     * \brief robotModelが追加された時のイベント
     * \since ver2.0
     * \param callback RobotModel型の引数をとる関数
     *
     */
    const Member &onRobotModelEntry(
        std::function<void WEBCFACE_CALL_FP(RobotModel)> callback) const;
    /*!
     * \brief funcが追加された時のイベント
     * \since ver2.0
     * \param callback Func型の引数をとる関数
     *
     */
    const Member &
    onFuncEntry(std::function<void WEBCFACE_CALL_FP(Func)> callback) const;
    /*!
     * \brief imageが追加されたときのイベント
     * \since ver2.0
     * \param callback Image型の引数をとる関数
     *
     */
    const Member &
    onImageEntry(std::function<void WEBCFACE_CALL_FP(Image)> callback) const;
    /*!
     * \brief viewが追加されたときのイベント
     * \since ver2.0
     * \param callback View型の引数をとる関数
     *
     */
    const Member &
    onViewEntry(std::function<void WEBCFACE_CALL_FP(View)> callback) const;
    /*!
     * \brief canvas3dが追加されたときのイベント
     * \since ver2.0
     * \param callback Canvas3D型の引数をとる関数
     *
     */
    const Member &onCanvas3DEntry(
        std::function<void WEBCFACE_CALL_FP(Canvas3D)> callback) const;
    /*!
     * \brief canvas2dが追加されたときのイベント
     * \since ver2.0
     * \param callback Canvas2D型の引数をとる関数
     *
     */
    const Member &onCanvas2DEntry(
        std::function<void WEBCFACE_CALL_FP(Canvas2D)> callback) const;
    /*!
     * \brief logが追加されたときのイベント
     * \since ver2.4
     * \param callback Log型の引数をとる関数
     *
     */
    const Member &
    onLogEntry(std::function<void WEBCFACE_CALL_FP(Log)> callback) const;

    /*!
     * \brief Memberがsync()したときのイベント
     * \since ver2.0
     * \param callback Member型の引数をとる関数
     *
     */
    const Member &
    onSync(std::function<void WEBCFACE_CALL_FP(Member)> callback) const;
    /*!
     * \brief Memberがsync()したときのイベント
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Member &onSync(F callback) const {
        return onSync(
            [callback = std::move(callback)](const auto &) { callback(); });
    }

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
    const std::string &libName() const;
    /*!
     * \brief このMemberが使っているWebCFaceのバージョン
     *
     */
    const std::string &libVersion() const;
    /*!
     * \brief このMemberのIPアドレス
     *
     */
    const std::string &remoteAddr() const;

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
     * \since ver2.0
     *
     * * 通常は約5秒に1回更新される。
     * * pingStatus() と同様、通信速度データのリクエストも行う。
     * * ver2.0〜 Client自身に対しても使用可能
     *
     * \param callback Member型の引数を取る関数
     * \sa pingStatus()
     */
    const Member &
    onPing(std::function<void WEBCFACE_CALL_FP(Member)> callback) const;
    /*!
     * \brief 通信速度が更新された時のイベント
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Member &onPing(F callback) const {
        return onPing(
            [callback = std::move(callback)](const auto &) { callback(); });
    }

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
extern template Log Member::log<Log, true>() const;

WEBCFACE_NS_END

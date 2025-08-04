#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "robot_link.h"
#include "field.h"
#include "components.h"

WEBCFACE_NS_BEGIN
namespace internal {
template <typename Component>
class DataSetBuffer;
} // namespace internal

/*!
 * \brief RobotModelの送受信データを表すクラス
 *
 * コンストラクタではなく Member::robotModel() を使って取得してください
 *
 * (1.9〜) Canvas3DComponentを継承しており origin(), color()
 * などを指定してCanvas3Dに追加することができる
 *
 */
class WEBCFACE_DLL RobotModel : protected Field {
    std::shared_ptr<internal::DataSetBuffer<RobotLink>> sb;

  public:
    RobotModel();
    RobotModel(const Field &base);
    RobotModel(const Field &base, const SharedString &field)
        : RobotModel(Field{base, field}) {}

    friend class Canvas3D;
    friend class TemporalCanvas3DComponent;
    friend internal::DataSetBuffer<RobotLink>;

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     * 
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     * 
     */
    RobotModel child(String field) const {
        return this->Field::child(static_cast<SharedString&>(field));
    }
    /*!
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    RobotModel child(int index) const {
        return this->Field::child(std::to_string(index));
    }
    /*!
     * child()と同じ
     * \since ver1.11
     * 
     * ver2.0〜 wstring対応, ver2.10〜 String 型で置き換え
     * 
     */
    RobotModel operator[](String field) const { return child(std::move(field)); }
    /*!
     * child()と同じ
     * \since ver1.11
     * \deprecated ver2.8〜
     */
    [[deprecated]]
    RobotModel operator[](int index) const {
        return child(std::to_string(index));
    }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    RobotModel parent() const { return this->Field::parent(); }

    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback RobotModel型の引数(thisが渡される)を1つ取る関数
     *
     */
    const RobotModel &
    onChange(std::function<void WEBCFACE_CALL_FP(RobotModel)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const RobotModel &onChange(F callback) const {
        return onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
    }
    /*!
     * \deprecated
     * ver1.11まではEventTarget::appendListener()でコールバックを追加できたが、
     * ver2.0からコールバックは1個のみになった。
     * 互換性のため残しているがonChange()と同じ
     *
     */
    template <typename T>
    [[deprecated]] void appendListener(T &&callback) const {
        onChange(std::forward<T>(callback));
    }

    /*!
     * \brief モデルを初期化
     * \since ver1.9
     */
    const RobotModel &init() const;
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    const RobotModel &operator<<(RobotLink rl) const;
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    template <typename T>
    const RobotModel &add(T &&rl) const {
        *this << std::forward<T>(rl);
        return *this;
    }
    /*!
     * \brief addで追加したモデルをセットする
     * \since ver1.9
     */
    const RobotModel &sync() const;

    /*!
     * \brief モデルをセットする
     *
     */
    const RobotModel &set(const std::vector<RobotLink> &v) const;
    /*!
     * \brief モデルをセットする
     *
     */
    const RobotModel &operator=(const std::vector<RobotLink> &v) const {
        this->set(v);
        return *this;
    }
    /*!
     * \brief モデルをリクエストする
     * \since ver1.7
     *
     */
    const RobotModel &request() const;
    /*!
     * \brief モデルを返す
     *
     */
    std::optional<std::vector<RobotLink>> tryGet() const;
    /*!
     * \brief モデルを返す
     *
     */
    std::vector<RobotLink> get() const {
        return tryGet().value_or(std::vector<RobotLink>{});
    }
    operator std::vector<RobotLink>() const { return get(); }
    /*!
     * \brief このフィールドにデータが存在すればtrue
     * \since ver2.1
     *
     * tryGet() などとは違って、実際のデータを受信しない。
     * リクエストも送信しない。
     *
     */
    bool exists() const;
    /*!
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime() に変更
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    const RobotModel &free() const;

    /*!
     * \brief これをCanvas3DComponentに変換
     * \since ver2.0
     */
    TemporalCanvas3DComponent toComponent3D() const;
    /*!
     * \brief Canvas3DComponentに変換 + 要素の移動
     *
     */
    TemporalCanvas3DComponent origin(const Transform &origin) const {
        return toComponent3D().origin(origin);
    }
    /*!
     * \brief Canvas3DComponentに変換 + 色の指定
     *
     */
    TemporalCanvas3DComponent color(ViewColor color) const {
        return toComponent3D().color(color);
    }
    /*!
     * \brief Canvas3DComponentに変換 + RobotModelの関節を設定
     * \sa TemporalCanvas3DComponent::angles
     */
    template <typename... Args>
    TemporalCanvas3DComponent angles(Args &&...args) const {
        return toComponent3D().angles(std::forward<Args>(args)...);
    }

    /*!
     * \brief RobotModelの参照先を比較
     * \since ver1.11
     */
    template <typename T,
              typename std::enable_if_t<std::is_same_v<T, RobotModel>,
                                        std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T,
              typename std::enable_if_t<std::is_same_v<T, RobotModel>,
                                        std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};

WEBCFACE_NS_END

#pragma once
#include <webcface/common/def.h>
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
class WEBCFACE_DLL RobotModel : protected Field,
                                public Canvas3DComponent {
    std::shared_ptr<internal::DataSetBuffer<RobotLink>> sb;

  public:
    RobotModel();
    RobotModel(const Field &base);
    RobotModel(const Field &base, const SharedString &field)
        : RobotModel(Field{base, field}) {}

    friend class Canvas3D;
    friend class Canvas3DComponent;
    friend internal::DataSetBuffer<RobotLink>;

    using Field::lastName;
    using Field::member;
    using Field::name;
    using Field::nameW;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     */
    RobotModel child(std::string_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField (wstring)
     * \since ver2.0
     */
    RobotModel child(std::wstring_view field) const {
        return this->Field::child(field);
    }
    /*!
     * \since ver1.11
     */
    RobotModel child(int index) const { return this->Field::child(index); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    RobotModel operator[](std::string_view field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver2.0
     */
    RobotModel operator[](std::wstring_view field) const {
        return child(field);
    }
    /*!
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    RobotModel operator[](const char *field) const { return child(field); }
    /*!
     * \since ver2.0
     */
    RobotModel operator[](const wchar_t *field) const { return child(field); }
    /*!
     * child()と同じ
     * \since ver1.11
     */
    RobotModel operator[](int index) const { return child(index); }
    /*!
     * \brief nameの最後のピリオドの前までを新しい名前とするField
     * \since ver1.11
     */
    RobotModel parent() const { return this->Field::parent(); }

  private:
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを取得
     * \since ver2.0
     */
    std::function<void(RobotModel)> &onChange();

  public:
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     */
    RobotModel &onChange(std::function<void(RobotModel)> callback);

    /*!
     * \brief モデルを初期化
     * \since ver1.9
     */
    RobotModel &init();
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    RobotModel &operator<<(const RobotLink &rl);
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    RobotModel &operator<<(RobotLink &&rl);
    /*!
     * \brief モデルにlinkを追加
     * \since ver1.9
     */
    template <typename T>
    RobotModel &add(T &&rl) {
        *this << std::forward<T>(rl);
        return *this;
    }
    /*!
     * \brief addで追加したモデルをセットする
     * \since ver1.9
     */
    RobotModel &sync();

    /*!
     * \brief モデルをセットする
     *
     */
    RobotModel &set(const std::vector<RobotLink> &v);
    /*!
     * \brief モデルをセットする
     *
     */
    RobotModel &operator=(const std::vector<RobotLink> &v) {
        this->set(v);
        return *this;
    }
    /*!
     * \brief モデルをリクエストする
     * \since ver1.7
     *
     */
    void request() const;
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
     * \brief syncの時刻を返す
     * \deprecated 1.7でMember::syncTime() に変更
     */
    [[deprecated]] std::chrono::system_clock::time_point time() const;

    /*!
     * \brief 値やリクエスト状態をクリア
     *
     */
    RobotModel &free();

    /*!
     * \brief RobotModelの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, RobotModel>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
};

WEBCFACE_NS_END

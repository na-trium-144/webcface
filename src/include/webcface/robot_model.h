#pragma once
#include "common/def.h"
#include "common/robot_model.h"
#include "field.h"
#include "event_target.h"
#include "canvas_data.h"

WEBCFACE_NS_BEGIN
namespace Internal {
struct ClientData;
template <typename Component>
class DataSetBuffer;
} // namespace Internal
class Member;

class RobotModel;
extern template class WEBCFACE_IMPORT EventTarget<RobotModel>;

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
                                public EventTarget<RobotModel>,
                                public Canvas3DComponent {
    std::shared_ptr<Internal::DataSetBuffer<RobotLink>> sb;

    void onAppend() const override final;

  public:
    RobotModel();
    RobotModel(const Field &base);
    RobotModel(const Field &base, std::u8string_view field)
        : RobotModel(Field{base, field}) {}
    RobotModel(const Field &base, FieldNameRef field)
        : RobotModel(Field{base, field}) {}

    friend class Canvas3D;
    friend Internal::DataSetBuffer<RobotLink>;

    using Field::lastName;
    using Field::member;
    using Field::name;
    /*!
     * \brief 「(thisの名前).(追加の名前)」を新しい名前とするField
     * \since ver1.11
     */
    RobotModel child(std::string_view field) const {
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
     * operator[](long, const char *)と解釈されるのを防ぐための定義
     * \since ver1.11
     */
    RobotModel operator[](const char *field) const { return child(field); }
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
    /*!
     * \brief RobotModelの参照先を比較
     * \since ver1.11
     *
     */
    template <typename T>
        requires std::same_as<T, RobotModel>
    bool operator!=(const T &other) const {
        return !(*this == other);
    }
};

WEBCFACE_NS_END

#pragma once
#include "webcface/component_view.h"
#include "webcface/geometry.h"
#include "webcface/transform.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace internal {
struct Canvas3DComponentData;
}

enum class Canvas3DComponentType {
    geometry = 0,
    robot_model = 1,
    // scatter = 2,
};

/*!
 * \brief Canvas3Dに表示する要素
 *
 * * ver2.0〜:
 * get専用(Canvas3DComponent)とset用(TemporalComponent)で分けている。
 *
 */
class WEBCFACE_DLL Canvas3DComponent {
    std::shared_ptr<internal::Canvas3DComponentData> msg_data;
    std::weak_ptr<internal::ClientData> data_w;
    int idx_for_type = 0;

    void checkData() const;

  public:
    /*!
     * msg_dataはnullptrになり、内容にアクセスしようとするとruntime_errorを投げる
     *
     */
    Canvas3DComponent();
    /*!
     * \param msg_data
     * \param data_w
     * \param idx_next 種類ごとの要素数のmap
     * InputRefの名前に使うidを決定するのに使う
     *
     */
    Canvas3DComponent(
        const std::shared_ptr<internal::Canvas3DComponentData> &msg_data,
        const std::weak_ptr<internal::ClientData> &data_w,
        std::unordered_map<Canvas3DComponentType, int> *idx_next);

    /*!
     * \brief そのcanvas3d内で一意のid
     * \since ver2.0
     *
     * 要素が増減したり順序が変わったりしなければ、
     * 同じ要素には常に同じidが振られる。
     *
     */
    std::string id() const;
    /*!
     * \since ver1.11
     */
    bool operator==(const Canvas3DComponent &other) const;
    /*!
     * \since ver1.11
     */
    bool operator!=(const Canvas3DComponent &other) const {
        return !(*this == other);
    }

    /*!
     * \brief 要素の種類
     *
     */
    Canvas3DComponentType type() const;
    /*!
     * \brief 要素の移動
     *
     */
    Transform origin() const;
    /*!
     * \brief 色
     *
     */
    ViewColor color() const;
    /*!
     * \brief geometryを取得
     *
     */
    std::optional<Geometry> geometry() const;
    /*!
     * \brief RobotModelを取得
     *
     */
    std::optional<RobotModel> robotModel() const;
};

/*!
 * \brief Canvas3Dを構築するときに使う一時的なCanvas3DComponent
 * \since ver2.0
 *
 */
class WEBCFACE_DLL TemporalCanvas3DComponent {
    std::unique_ptr<internal::Canvas3DComponentData> msg_data;

  public:
    /*!
     * msg_dataはnullptrになる
     *
     */
    explicit TemporalCanvas3DComponent(std::nullptr_t = nullptr);
    /*!
     * msg_dataを初期化する
     *
     */
    explicit TemporalCanvas3DComponent(Canvas3DComponentType type);
    TemporalCanvas3DComponent(const TemporalCanvas3DComponent &other);
    TemporalCanvas3DComponent &
    operator=(const TemporalCanvas3DComponent &other);
    ~TemporalCanvas3DComponent() noexcept;

    /*!
     * \brief AnonymousFuncの名前を確定
     *
     * 現状Canvas3DにFuncを使う要素はないのでなにもしない
     *
     * \param data
     * \param view_name viewの名前
     * \param idx_next 種類ごとの要素数のmap
     * InputRefの名前に使うidを決定するのに使う
     *
     */
    TemporalCanvas3DComponent &
    lockTmp(const std::shared_ptr<internal::ClientData> &data,
            const SharedString &view_name,
            std::unordered_map<Canvas3DComponentType, int> *idx_next = nullptr);

    /*!
     * \brief 要素の移動
     *
     */
    TemporalCanvas3DComponent &origin(const Transform &origin);
    /*!
     * \brief 色
     *
     */
    TemporalCanvas3DComponent &color(ViewColor color);
    /*!
     * \brief geometryをセット
     *
     */
    TemporalCanvas3DComponent &geometry(const Geometry &g);
    TemporalCanvas3DComponent &robotModel(const RobotModel &field);
    /*!
     * \brief RobotModelの関節をまとめて設定
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    TemporalCanvas3DComponent &
    angles(const std::unordered_map<std::string, double> &angles);
    /*!
     * \brief RobotModelの関節をまとめて設定 (wstring)
     * \since ver2.0
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    TemporalCanvas3DComponent &
    angles(const std::unordered_map<std::wstring, double> &angles);
    /*!
     * \brief RobotModelの関節を設定
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    TemporalCanvas3DComponent &angle(const std::string &joint_name,
                                     double angle);
    /*!
     * \brief RobotModelの関節を設定 (wstring)
     * \since ver2.0
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    TemporalCanvas3DComponent &angle(const std::wstring &joint_name,
                                     double angle);
};

WEBCFACE_NS_END

#pragma once
#include "webcface/component_view.h"
#include "webcface/geometry.h"
#include "webcface/transform.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
namespace Message {
struct Canvas3DComponent;
}

enum class Canvas3DComponentType {
    geometry = 0,
    robot_model = 1,
    // scatter = 2,
};

/*!
 * \brief Canvas3Dに表示する要素
 *
 */
class WEBCFACE_DLL Canvas3DComponent {
    std::weak_ptr<Internal::ClientData> data_w;

    Canvas3DComponentType type_ = Canvas3DComponentType::geometry;
    Transform origin_;
    ViewColor color_ = ViewColor::inherit;
    std::optional<Geometry> geometry_;
    std::optional<FieldBase> field_base_;
    std::unordered_map<std::size_t, double> angles_;

  public:
    Canvas3DComponent() = default;
    Canvas3DComponent(Canvas3DComponentType type, const Transform &origin,
                      ViewColor color, std::optional<Geometry> &&geometry,
                      std::optional<FieldBase> &&field_base,
                      std::unordered_map<std::size_t, double> &&angles)
        : type_(type), origin_(origin), color_(color),
          geometry_(std::move(geometry)), field_base_(std::move(field_base)),
          angles_(std::move(angles)) {}
    Canvas3DComponent(const Canvas3DComponent &vc,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : Canvas3DComponent(vc) {
        this->data_w = data_w;
    }
    // explicit Canvas3DComponent(const Common::Canvas3DComponentBase &vc)
    //     : Common::Canvas3DComponentBase(vc), data_w() {}
    Canvas3DComponent(Canvas3DComponentType type,
                      const std::weak_ptr<Internal::ClientData> &data_w)
        : data_w(data_w), type_(type) {}
    explicit Canvas3DComponent(Canvas3DComponentType type)
        : data_w(), type_(type) {}

    /*!
     * \brief AnonymousFuncをFuncオブジェクトにlock
     *
     * 現状Funcをセットする要素無いのでなにもしない
     *
     */
    Canvas3DComponent &
    lockTmp(const std::weak_ptr<Internal::ClientData> & /*data_w*/,
            const std::u8string & /*field_id*/) {
        return *this;
    }

    Message::Canvas3DComponent toMessage() const;
    Canvas3DComponent(const Message::Canvas3DComponent &cc);

    /*!
     * \since ver1.11
     */
    bool operator==(const Canvas3DComponent &other) const {
        return /*id() == other.id() && */ type_ == other.type_ &&
               origin_ == other.origin_ && color_ == other.color_ &&
               geometry_ == other.geometry_ &&
               field_base_ == other.field_base_ && angles_ == other.angles_;
    }
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
    Canvas3DComponentType type() const { return type_; }
    /*!
     * \brief 要素の移動
     *
     */
    Transform origin() const { return origin_; }
    /*!
     * \brief 要素の移動
     *
     */
    Canvas3DComponent &origin(const Transform &origin) {
        origin_ = origin;
        return *this;
    }
    /*!
     * \brief 色
     *
     */
    ViewColor color() const { return color_; }
    /*!
     * \brief 色
     *
     */
    Canvas3DComponent &color(ViewColor color) {
        color_ = color;
        return *this;
    }
    /*!
     * \brief geometryを取得
     *
     */
    const std::optional<Geometry> &geometry() const { return geometry_; };
    /*!
     * \brief geometryをセット
     *
     */
    Canvas3DComponent &geometry(const Geometry &g) {
        geometry_.emplace(g);
        return *this;
    };
    /*!
     * \brief RobotModelを取得
     *
     */
    std::optional<RobotModel> robotModel() const;
    Canvas3DComponent &robotModel(const RobotModel &field);
    /*!
     * \brief RobotModelの関節をまとめて設定
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    Canvas3DComponent &
    angles(const std::unordered_map<std::string, double> &angles);
    /*!
     * \brief RobotModelの関節をまとめて設定 (wstring)
     * \since ver2.0
     * \param angles RobotJointの名前と角度のリスト
     *
     */
    Canvas3DComponent &
    angles(const std::unordered_map<std::wstring, double> &angles);
    /*!
     * \brief RobotModelの関節を設定
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    Canvas3DComponent &angle(const std::string &joint_name, double angle);
    /*!
     * \brief RobotModelの関節を設定 (wstring)
     * \since ver2.0
     * \param joint_name RobotJointの名前
     * \param angle 角度
     *
     */
    Canvas3DComponent &angle(const std::wstring &joint_name, double angle);
};


WEBCFACE_NS_END

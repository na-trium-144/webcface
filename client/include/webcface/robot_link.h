#pragma once
#include "webcface/component_view.h"
#include "webcface/encoding/encoding.h"
#include "webcface/geometry.h"
#include "webcface/transform.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace message {
struct RobotLink;
}

enum class RobotJointType {
    fixed_absolute = 0,
    fixed = 1,
    rotational = 2,
    prismatic = 3,
};
struct RobotJoint {
    SharedString name;
    SharedString parent_name;
    RobotJointType type;
    Transform origin;
    double angle = 0;
};
inline namespace robot_joints {
/*!
 * \brief 親リンクをもたず座標を定義する
 *
 * ベースのリンクなどに使う
 * \param origin 子リンクの絶対座標
 *
 */
inline RobotJoint fixedAbsolute(const Transform &origin) {
    return RobotJoint{nullptr, nullptr, RobotJointType::fixed_absolute, origin,
                      0};
}
/*!
 * \brief 親リンクをもたず座標を定義する
 *
 * ベースのリンクなどに使う
 * \param origin 子リンクの絶対座標
 *
 */
inline RobotJoint fixedAbsolute(const Point &origin) {
    return fixedAbsolute(Transform{origin, {}});
}
/*!
 * \brief 固定された関節
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 *
 */
inline RobotJoint fixedJoint(std::string_view parent_name,
                             const Transform &origin) {
    return RobotJoint{nullptr, SharedString::encode(parent_name),
                      RobotJointType::fixed, origin, 0};
}
/*!
 * \brief 固定された関節 (wstring)
 * \since ver2.0
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 *
 */
inline RobotJoint fixedJoint(std::wstring_view parent_name,
                             const Transform &origin) {
    return RobotJoint{nullptr, SharedString::encode(parent_name),
                      RobotJointType::fixed, origin, 0};
}
/*!
 * \brief 固定された関節
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 */
inline RobotJoint fixedJoint(std::string_view parent_name,
                             const Point &origin) {
    return fixedJoint(parent_name, Transform{origin, {}});
}
/*!
 * \brief 固定された関節 (wstring)
 * \since ver2.0
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 */
inline RobotJoint fixedJoint(std::wstring_view parent_name,
                             const Point &origin) {
    return fixedJoint(parent_name, Transform{origin, {}});
}
/*!
 * \brief 回転関節
 *
 * originのz軸を中心に回転する関節。
 * \param name 関節の名前
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 * \param angle 初期状態の回転角
 *
 */
inline RobotJoint rotationalJoint(std::string_view name,
                                  std::string_view parent_name,
                                  const Transform &origin, double angle = 0) {
    return RobotJoint{SharedString::encode(name),
                      SharedString::encode(parent_name),
                      RobotJointType::rotational, origin, angle};
}
/*!
 * \brief 回転関節 (wstring)
 * \since ver2.0
 *
 * originのz軸を中心に回転する関節。
 * \param name 関節の名前
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 * \param angle 初期状態の回転角
 *
 */
inline RobotJoint rotationalJoint(std::wstring_view name,
                                  std::wstring_view parent_name,
                                  const Transform &origin, double angle = 0) {
    return RobotJoint{SharedString::encode(name),
                      SharedString::encode(parent_name),
                      RobotJointType::rotational, origin, angle};
}
/*!
 * \brief 直動関節
 *
 * originのz軸方向に直線運動する関節。
 * \param name 関節の名前
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 * \param angle 初期状態の回転角(移動距離)
 *
 */
inline RobotJoint prismaticJoint(std::string_view name,
                                 std::string_view parent_name,
                                 const Transform &origin, double angle = 0) {
    return RobotJoint{SharedString::encode(name),
                      SharedString::encode(parent_name),
                      RobotJointType::prismatic, origin, angle};
}
/*!
 * \brief 直動関節 (wstring)
 * \since ver2.0
 *
 * originのz軸方向に直線運動する関節。
 * \param name 関節の名前
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 * \param angle 初期状態の回転角(移動距離)
 *
 */
inline RobotJoint prismaticJoint(std::wstring_view name,
                                 std::wstring_view parent_name,
                                 const Transform &origin, double angle = 0) {
    return RobotJoint{SharedString::encode(name),
                      SharedString::encode(parent_name),
                      RobotJointType::prismatic, origin, angle};
}
} // namespace robot_joints

namespace RobotJoints = robot_joints; // 〜ver1.11

struct WEBCFACE_DLL RobotLink {
    SharedString name;
    RobotJoint joint;
    Geometry geometry;
    ViewColor color;
    RobotLink() = default;
    RobotLink(const message::RobotLink &m,
              const std::vector<SharedString> &link_names);
    message::RobotLink
    toMessage(const std::vector<SharedString> &link_names) const;
    /*!
     * \since ver2.0
     */
    RobotLink(const SharedString &name, const RobotJoint &joint,
              const Geometry &geometry, ViewColor color)
        : name(name), joint(joint), geometry(geometry), color(color) {}
    /*!
     * \param name リンクの名前
     * \param joint 親リンクとの接続方法
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     */
    RobotLink(std::string_view name, const RobotJoint &joint,
              const Geometry &geometry, ViewColor color = ViewColor::inherit)
        : RobotLink(SharedString::encode(name), joint, geometry, color) {}
    /*!
     * \since ver2.0
     * \param name リンクの名前
     * \param joint 親リンクとの接続方法
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     */
    RobotLink(std::wstring_view name, const RobotJoint &joint,
              const Geometry &geometry, ViewColor color = ViewColor::inherit)
        : RobotLink(SharedString::encode(name), joint, geometry, color) {}
    /*!
     * ベースのリンクではjointを省略可能
     * (fixedAbsolute({0, 0, 0})になる)
     * \param name リンクの名前
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     */
    RobotLink(std::string_view name, const Geometry &geometry,
              ViewColor color = ViewColor::inherit)
        : RobotLink(name, fixedAbsolute({0, 0, 0}), geometry, color) {}
    /*!
     * ベースのリンクではjointを省略可能
     * (fixedAbsolute({0, 0, 0})になる)
     * \since ver2.0
     * \param name リンクの名前
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     */
    RobotLink(std::wstring_view name, const Geometry &geometry,
              ViewColor color = ViewColor::inherit)
        : RobotLink(name, fixedAbsolute({0, 0, 0}), geometry, color) {}
};
WEBCFACE_NS_END

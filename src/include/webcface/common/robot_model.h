#pragma once
#include "def.h"
#include "view.h"
#include "canvas3d.h"
#include <string>
#include <concepts>

WEBCFACE_NS_BEGIN
inline namespace Common {

enum class RobotJointType {
    fixed_absolute = 0,
    fixed = 1,
    rotational = 2,
    prismatic = 3,
};
struct RobotJoint {
    std::string name;
    std::string parent_name;
    RobotJointType type;
    Transform origin;
    double angle = 0;
};
inline namespace RobotJoints {
/*!
 * \brief 親リンクをもたず座標を定義する
 *
 * ベースのリンクなどに使う
 * \param origin 子リンクの絶対座標
 *
 */
inline RobotJoint fixedAbsolute(const Transform &origin) {
    return RobotJoint{"", "", RobotJointType::fixed_absolute, origin, 0};
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
inline RobotJoint fixedJoint(const std::string &parent_name,
                             const Transform &origin) {
    return RobotJoint{"", parent_name, RobotJointType::fixed, origin, 0};
}
/*!
 * \brief 固定された関節
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
 */
inline RobotJoint fixedJoint(const std::string &parent_name,
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
inline RobotJoint rotationalJoint(const std::string &name,
                                  const std::string &parent_name,
                                  const Transform &origin, double angle = 0) {
    return RobotJoint{name, parent_name, RobotJointType::rotational, origin,
                      angle};
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
inline RobotJoint prismaticJoint(const std::string &name,
                                 const std::string &parent_name,
                                 const Transform &origin, double angle = 0) {
    return RobotJoint{name, parent_name, RobotJointType::prismatic, origin,
                      angle};
}
} // namespace RobotJoints

struct RobotLink {
    std::string name;
    RobotJoint joint;
    Geometry geometry;
    ViewColor color;
    RobotLink() = default;
    /*!
     * \param name リンクの名前
     * \param joint 親リンクとの接続方法
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     */
    RobotLink(const std::string &name, const RobotJoint &joint,
              const Geometry &geometry, ViewColor color = ViewColor::inherit)
        : name(name), joint(joint), geometry(geometry), color(color) {}
    /*!
     * ベースのリンクではjointを省略可能
     * (fixedAbsolute({0, 0, 0})になる)
     * \param name リンクの名前
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     */
    RobotLink(const std::string &name, const Geometry &geometry,
              ViewColor color = ViewColor::inherit)
        : RobotLink(name, fixedAbsolute({0, 0, 0}), geometry, color) {}
};
} // namespace Common
WEBCFACE_NS_END

#pragma once
#include "def.h"
#include "view.h"
#include "canvas3d.h"
#include <string>

namespace WEBCFACE_NS {
inline namespace Common {

enum class RobotJointType {
    fixed = 0,
    rotational = 1,
    prismatic = 2,
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
 * \brief 固定された関節
 * \param parent_name 親リンクの名前
 * \param origin 親リンクの座標系で子リンクの原点
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
    return RobotJoint{"", parent_name, RobotJointType::fixed,
                      Transform{origin, {}}, 0};
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
     * \param name リンクの名前
     * \param geometry リンクの形状 (表示用)
     * \param color 色 (表示用)
     *
     */
    RobotLink(const std::string &name, const Geometry &geometry,
              ViewColor color = ViewColor::inherit)
        : RobotLink(name, {}, geometry, color) {}
};
} // namespace Common
} // namespace WEBCFACE_NS

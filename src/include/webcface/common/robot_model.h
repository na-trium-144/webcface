#pragma once
#include "def.h"
#include <string>
#include <optional>
#include <vector>
#include <array>
#include "view.h"

namespace WEBCFACE_NS {
inline namespace Common {
/*!
 * \brief 3次元の座標と回転
 *
 */
class Transform {
    std::array<double, 3> pos_;
    std::array<double, 3> rot_euler_;

  public:
    Transform() : pos_({0, 0, 0}), rot_euler_({0, 0, 0}) {}
    Transform(const std::array<double, 3> &pos,
              const std::array<double, 3> &rot_euler)
        : pos_(pos), rot_euler_(rot_euler){};
    std::array<double, 3> pos() const { return pos_; }
    std::array<double, 3> rot() const { return rot_euler_; }
};

enum class RobotJointType {
    fixed = 0,
};
struct RobotJoint {
    std::string name;
    std::string parent_name;
    RobotJointType type;
    Transform origin;
    double angle = 0;
};
enum class RobotGeometryType {
    none = 0,
    line = 1,
    plain = 2,
    box = 3,
};
struct RobotGeometry {
    RobotGeometryType type;
    Transform origin;
    std::vector<double> properties;
};
struct RobotLink {
    std::string name;
    RobotJoint joint;
    RobotGeometry geometry;
    ViewColor color;
};
} // namespace Common
} // namespace WEBCFACE_NS

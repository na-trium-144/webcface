#pragma once
#include "def.h"
#include "view.h"
#include "canvas3d.h"
#include <string>

namespace WEBCFACE_NS {
inline namespace Common {

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
inline namespace RobotJoints {
inline RobotJoint fixedJoint(const std::string &name,
                             const std::string &parent_name,
                             const Transform &origin) {
    return RobotJoint{name, parent_name, RobotJointType::fixed, origin, 0};
}
} // namespace RobotJoints

struct RobotLink {
    std::string name;
    RobotJoint joint;
    Geometry geometry;
    ViewColor color;
};
} // namespace Common
} // namespace WEBCFACE_NS

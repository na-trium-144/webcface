#pragma once
#include "def.h"
#include <string>
#include <optional>
#include <vector>
#include <array>
#include <stdexcept>
#include "view.h"

namespace WEBCFACE_NS {
inline namespace Common {
/*!
 * \brief 3次元の座標と回転
 *
 * 内部ではx, y, zの座標とz-y-x系のオイラー角で保持している。
 *
 */
class Transform {
    std::array<double, 3> pos_;
    std::array<double, 3> rot_;

  public:
    /**
     * \param pos x, y, z 座標
     * \param rot オイラー角 (z, y, x の順)
     *
     */
    Transform(const std::array<double, 3> &pos = {0, 0, 0},
              const std::array<double, 3> &rot = {0, 0, 0})
        : pos_(pos), rot_(rot){};
    Transform(double x, double y = 0, double z = 0, double z_angle = 0,
              double y_angle = 0, double x_angle = 0)
        : Transform(std::array<double, 3>{x, y, z},
                    std::array<double, 3>{z_angle, y_angle, x_angle}) {}
    std::array<double, 3> pos() const { return pos_; }
    std::array<double, 3> rot() const { return rot_; }
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
inline namespace RobotJoints {
inline RobotJoint fixedJoint(const std::string &name,
                             const std::string &parent_name,
                             const Transform &origin) {
    return RobotJoint{name, parent_name, RobotJointType::fixed, origin, 0};
}
} // namespace RobotJoints

enum class RobotGeometryType {
    none = 0,
    line = 1,
    plane = 2,
    box = 3,
};
struct RobotGeometry {
    RobotGeometryType type;
    Transform origin;
    std::vector<double> properties;
    RobotGeometry() : type(RobotGeometryType::none), origin(), properties() {}
    RobotGeometry(RobotGeometryType type, const Transform &origin,
                  const std::vector<double> &properties)
        : type(type), origin(origin), properties(properties) {}
};
inline namespace RobotGeometries {
struct Line : RobotGeometry {
    Line(const Transform &origin, const Transform &end)
        : RobotGeometry(RobotGeometryType::line, origin,
                        {end.pos()[0], end.pos()[1], end.pos()[2]}) {}
    Line(const RobotGeometry &rg) : RobotGeometry(rg) {
        if (properties.size() != 3) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    Transform end() const {
        return Transform{{properties[0], properties[1], properties[2]},
                         {0, 0, 0}};
    }
};
struct Plane : RobotGeometry {
    Plane(const Transform &origin, double width, double height)
        : RobotGeometry(RobotGeometryType::plane, origin, {width, height}) {}
    Plane(const RobotGeometry &rg) : RobotGeometry(rg) {
        if (properties.size() != 2) {
            throw std::invalid_argument("number of properties does not match");
        }
    }
    double width() const { return properties[0]; }
    double height() const { return properties[1]; }
};
} // namespace RobotGeometries

struct RobotLink {
    std::string name;
    RobotJoint joint;
    RobotGeometry geometry;
    ViewColor color;
};
} // namespace Common
} // namespace WEBCFACE_NS

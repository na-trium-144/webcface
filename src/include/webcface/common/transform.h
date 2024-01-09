#pragma once
#include "def.h"
#include <array>

namespace WEBCFACE_NS {
inline namespace Common {
/*!
 * \brief 3次元の座標
 *
 */
class Point {
  protected:
    std::array<double, 3> pos_;

  public:
    Point(const std::array<double, 3> &pos = {0, 0, 0}) : pos_(pos) {}
    Point(double x, double y, double z)
        : Point(std::array<double, 3>{x, y, z}) {}
    std::array<double, 3> pos() const { return pos_; }
    bool operator==(const Point &rhs) const { return pos_ == rhs.pos_; }
    bool operator!=(const Point &rhs) const { return !(*this == rhs); }
};

/*!
 * \brief 3次元の座標と回転
 *
 * 内部ではx, y, zの座標とz-y-x系のオイラー角で保持している。
 *
 */
class Transform : public Point {
  protected:
    std::array<double, 3> rot_;

  public:
    /**
     * \param pos x, y, z 座標
     * \param rot オイラー角 (z, y, x の順)
     *
     */
    Transform() = default;
    Transform(const Point &pos, const std::array<double, 3> &rot)
        : Point(pos), rot_(rot){};
    Transform(double x, double y, double z, double z_angle, double y_angle,
              double x_angle)
        : Transform(std::array<double, 3>{x, y, z},
                    std::array<double, 3>{z_angle, y_angle, x_angle}) {}
    std::array<double, 3> rot() const { return rot_; }

    bool operator==(const Transform &rhs) const {
        return pos_ == rhs.pos_ && rot_ == rhs.rot_;
    }
    bool operator!=(const Transform &rhs) const { return !(*this == rhs); }
};
} // namespace Common
} // namespace WEBCFACE_NS

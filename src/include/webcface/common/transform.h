#pragma once
#include "def.h"
#include <array>

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

    bool operator==(const Transform &rhs) const {
        return pos_ == rhs.pos_ && rot_ == rhs.rot_;
    }
    bool operator!=(const Transform &rhs) const { return !(*this == rhs); }
};
} // namespace Common
} // namespace WEBCFACE_NS

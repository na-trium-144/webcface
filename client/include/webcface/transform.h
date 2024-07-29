#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <array>

WEBCFACE_NS_BEGIN
/*!
 * \brief 3次元 or 2次元の座標
 *
 * 2次元はver1.6から実装
 *
 */
class Point {
  protected:
    std::array<double, 3> pos_;

  public:
    Point(const std::array<double, 3> &pos = {0, 0, 0}) : pos_(pos) {}
    /*!
     * ver1.6から追加
     *
     */
    Point(const std::array<double, 2> &pos) : pos_({pos[0], pos[1], 0}) {}
    Point(double x, double y, double z = 0)
        : Point(std::array<double, 3>{x, y, z}) {}
    std::array<double, 3> pos() const { return pos_; }
    /*!
     * ver1.6から追加
     *
     */
    std::array<double, 3> &pos() { return pos_; }
    /*!
     * ver1.6から追加
     *
     */
    double pos(std::size_t i) const { return pos_.at(i); }
    /*!
     * ver1.6から追加
     *
     */
    double &pos(std::size_t i) { return pos_.at(i); }

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
    Transform() : Point(), rot_({0, 0, 0}) {}
    /**
     * \param pos x, y, z 座標
     * \param rot オイラー角 (z, y, x の順)
     *
     */
    Transform(const Point &pos, const std::array<double, 3> &rot)
        : Point(pos), rot_(rot){};
    /**
     * 2次元用、ver1.6から追加
     * \param pos x, y 座標
     * \param rot 回転角(z)
     *
     */
    Transform(const Point &pos, double rot) : Transform(pos, {rot, 0, 0}){};
    Transform(double x, double y, double z, double z_angle, double y_angle,
              double x_angle)
        : Transform(std::array<double, 3>{x, y, z},
                    std::array<double, 3>{z_angle, y_angle, x_angle}) {}
    /*!
     * ver1.6から追加
     * 2次元座標用
     *
     */
    Transform(double x, double y) : Transform(x, y, 0, 0, 0, 0) {}

    std::array<double, 3> rot() const { return rot_; }
    /*!
     * ver1.6から追加
     *
     */
    double rot(std::size_t i) const { return rot_.at(i); }
    /*!
     * ver1.6から追加
     *
     */
    double &rot(std::size_t i) { return rot_.at(i); }
    /*!
     * ver1.6から追加
     *
     */
    std::array<double, 3> &rot() { return rot_; }

    bool operator==(const Transform &rhs) const {
        return pos_ == rhs.pos_ && rot_ == rhs.rot_;
    }
    bool operator!=(const Transform &rhs) const { return !(*this == rhs); }
};

inline Transform identity() { return Transform{}; }

WEBCFACE_NS_END

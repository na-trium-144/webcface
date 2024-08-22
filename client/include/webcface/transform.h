#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <array>

WEBCFACE_NS_BEGIN
/*!
 * \brief 3次元 or 2次元の座標を表すクラス。
 * \since ver1.4
 *
 * 2次元はver1.6から実装
 *
 */
class Point {
  protected:
    std::array<double, 3> pos_;

  public:
    /*!
     * \brief 3次元座標を初期化
     *
     */
    Point(const std::array<double, 3> &pos = {0, 0, 0}) : pos_(pos) {}
    /*!
     * \brief 2次元座標を初期化
     * \since ver1.6
     */
    Point(const std::array<double, 2> &pos) : pos_({pos[0], pos[1], 0}) {}
    /*!
     * \brief 2or3次元座標を初期化
     *
     */
    Point(double x, double y, double z = 0)
        : Point(std::array<double, 3>{x, y, z}) {}
    /*!
     * \brief 3次元座標を取得
     *
     */
    std::array<double, 3> pos() const { return pos_; }
    /*!
     * \brief 3次元座標を取得・変更
     * \since ver1.6
     */
    std::array<double, 3> &pos() { return pos_; }
    /*!
     * \brief 座標を取得
     * \since ver1.6
     * \param index 0→x, 1→y, 2→z
     */
    double pos(std::size_t index) const { return pos_.at(index); }
    /*!
     * \brief 座標を取得・変更
     * \since ver1.6
     * \param index 0→x, 1→y, 2→z
     */
    double &pos(std::size_t i) { return pos_.at(i); }

    bool operator==(const Point &rhs) const { return pos_ == rhs.pos_; }
    bool operator!=(const Point &rhs) const { return !(*this == rhs); }
};

/*!
 * \brief 3次元の座標と回転
 * \since ver1.4
 *
 * 内部ではx, y, zの座標とz-y-x系のオイラー角で保持している。
 *
 */
class Transform : public Point {
  protected:
    std::array<double, 3> rot_;

  public:
    Transform() : Point(), rot_({0, 0, 0}) {}
    /*!
     * \brief 3次元の座標と回転を初期化
     * \param pos x, y, z 座標
     * \param rot オイラー角 (z, y, x の順)
     *
     */
    Transform(const Point &pos, const std::array<double, 3> &rot)
        : Point(pos), rot_(rot) {};
    /*!
     * \brief 2次元の座標と回転を初期化
     * \since ver1.6
     * \param pos x, y 座標
     * \param rot 回転角(z)
     *
     */
    Transform(const Point &pos, double rot) : Transform(pos, {rot, 0, 0}) {};
    /*!
     * \brief 3次元の座標と回転を初期化
     *
     */
    Transform(double x, double y, double z, double z_angle, double y_angle,
              double x_angle)
        : Transform(std::array<double, 3>{x, y, z},
                    std::array<double, 3>{z_angle, y_angle, x_angle}) {}
    /*!
     * \brief 2次元の座標を初期化
     * \since ver1.6
     */
    Transform(double x, double y) : Transform(x, y, 0, 0, 0, 0) {}

    /*!
     * \brief 3次元の回転を取得
     *
     */
    std::array<double, 3> rot() const { return rot_; }
    /*!
     * \brief 3次元の回転を取得・変更
     * \since ver1.6
     */
    std::array<double, 3> &rot() { return rot_; }
    /*!
     * \brief 回転を取得
     * \since ver1.6
     * \param index 0→z, 1→y, 2→x
     */
    double rot(std::size_t index) const { return rot_.at(index); }
    /*!
     * \brief 回転を取得・変更
     * \since ver1.6
     * \param index 0→z, 1→y, 2→x
     */
    double &rot(std::size_t index) { return rot_.at(index); }

    bool operator==(const Transform &rhs) const {
        return pos_ == rhs.pos_ && rot_ == rhs.rot_;
    }
    bool operator!=(const Transform &rhs) const { return !(*this == rhs); }
};

/*!
 * \brief 移動なし、回転なしのTransform
 * \since ver1.4
 */
inline Transform identity() { return Transform{}; }

WEBCFACE_NS_END

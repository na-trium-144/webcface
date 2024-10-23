#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <array>
#include <optional>
#include <cassert>

WEBCFACE_NS_BEGIN
/*!
 * \brief 3次元 or 2次元の座標を表すクラス。
 * \since ver1.4
 *
 * (ver1.6〜) 2次元の座標を表す場合にも使われ、その場合z=0とする。
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
 * \brief オイラー角の回転順序
 * \since ver2.5
 *
 * * 右手系の座標系で、
 * 内的回転(intrinsic rotation)でz軸,y軸,x軸の順に回転させる系
 * = 外的回転(extrinsic rotation)でX軸,Y軸,Z軸の順に回転させる系
 * = 回転行列がZ(α)Y(β)X(γ)と表される系
 * を、 AxisSequence::ZYX と表記する。
 * * ver2.4までの実装はすべてZYXで、現在もWebCFaceの内部表現は基本的にZYXの系である。
 * * またWebCFaceのインタフェースでオイラー角の回転角を指定する場合、
 * 軸の指定順は内的回転を指す。(AxisSequenceにおける左から右の並び順と一致。)
 *
 */
enum class AxisSequence {
    ZXZ,
    XYX,
    YZY,
    ZYZ,
    XZX,
    YXY,
    XYZ,
    YZX,
    ZXY,
    XZY,
    ZYX,
    YXZ,
};

/*!
 * \brief 3次元の回転を表すクラス
 * \since ver2.5
 *
 * * 内部では z軸,y軸,x軸の順に回転させる系のオイラー角(Tait-Bryan角)
 * または3x3回転行列(ver2.5〜)
 * で保持している。
 * * todo: 回転行列の代わりにクォータニオンを使うほうが良かったかも
 * * 送受信時にはすべてこのzyxのオイラー角に変換される。
 * * 2次元の回転を表すのにも使われ、
 * その場合オイラー角 rot() の最初の要素(=z軸周りの回転)を使って回転を表し、
 * 残りの要素(x,y軸周りの回転)を0とする。
 *
 * \sa AxisSequence
 */
class Rotation {
  protected:
    // rot_とrmat_のどちらかには必ず値が入っている。
    // 両方に値が入っているなら両方とも同じ回転を表現していなければならない。
    mutable std::optional<std::array<double, 3>> rot_;
    mutable std::optional<std::array<std::array<double, 3>, 3>> rmat_;

    Rotation(const std::optional<std::array<double, 3>> &rot,
             const std::optional<std::array<std::array<double, 3>, 3>> &rmat)
        : rot_(rot), rmat_(rmat) {}

  public:
    friend class Transform;

    /*!
     * \brief オイラー角から回転行列への変換
     * \since ver2.5
     */
    static WEBCFACE_DLL std::array<std::array<double, 3>, 3>
    eulerToMatrix(const std::array<double, 3> &rot, AxisSequence axis);
    /*!
     * \brief 回転行列からオイラー角への変換
     * \since ver2.5
     */
    static WEBCFACE_DLL std::array<double, 3>
    matrixToEuler(const std::array<std::array<double, 3>, 3> &rmat,
                  AxisSequence axis);

    /*!
     * \brief クォータニオンから回転行列への変換
     * \since ver2.5
     *
     * クォータニオンは w, x, y, z の順で表す。
     *
     */
    static WEBCFACE_DLL std::array<std::array<double, 3>, 3>
    quaternionToMatrix(const std::array<double, 4> &quat);
    /*!
     * \brief 回転行列からクォータニオンへの変換
     * \since ver2.5
     */
    static WEBCFACE_DLL std::array<double, 4>
    matrixToQuaternion(const std::array<std::array<double, 3>, 3> &rmat);

    /*!
     * \brief 回転軸と角度からクォータニオンに変換
     * \since ver2.5
     */
    static WEBCFACE_DLL std::array<double, 4>
    axisAngleToQuaternion(const std::array<double, 3> &axis, double angle);
    /*!
     * \brief クォータニオンから回転軸と角度に変換
     * \since ver2.5
     */
    static WEBCFACE_DLL std::pair<std::array<double, 3>, double>
    quaternionToAxisAngle(const std::array<double, 4> &quat);

    Rotation()
        : rot_({0, 0, 0}), rmat_({{
                               {{1, 0, 0}},
                               {{0, 1, 0}},
                               {{0, 0, 1}},
                           }}) {}

    template <AxisSequence axis>
    friend Rotation rotEuler(const std::array<double, 3> &rot);
    friend Rotation
    rotMatrix(const std::array<std::array<double, 3>, 3> &matrix);

    /*!
     * \brief 3次元の回転をオイラー角として取得
     * \tparam axis 回転軸の順序 (ver2.5〜)
     *  ver2.4までは z, y, x の順しか指定できない
     *
     */
    template <AxisSequence axis = AxisSequence::ZYX>
    std::array<double, 3> rot() const {
        if constexpr (axis == AxisSequence::ZYX) {
            if (!rot_) {
                assert(rmat_);
                rot_.emplace(matrixToEuler(*rmat_, AxisSequence::ZYX));
            }
            return *rot_;
        } else {
            return matrixToEuler(rotMatrix(), axis);
        }
    }

    /*!
     * \brief 3次元の回転をZYX順のオイラー角として取得
     * \since ver1.6
     * \param index 0→z, 1→y, 2→x
     */
    double rot(std::size_t index) const { return rot().at(index); }

    /*!
     * \brief 回転行列を取得
     * \since ver2.5
     */
    const std::array<std::array<double, 3>, 3> &rotMatrix() const {
        if (!rmat_) {
            assert(rot_);
            rmat_.emplace(eulerToMatrix(*rot_, AxisSequence::ZYX));
        }
        return *rmat_;
    }
    /*!
     * \brief 回転行列の要素を取得
     * \since ver2.5
     */
    double rotMatrix(std::size_t row, std::size_t col) const {
        return rotMatrix().at(row).at(col);
    }

    /*!
     * \brief クォータニオンとして取得
     * \since ver2.5
     */
    std::array<double, 4> rotQuat() const {
        return matrixToQuaternion(rotMatrix());
    }
    /*!
     * \brief 回転軸と角度として取得
     * \since ver2.5
     */
    std::pair<std::array<double, 3>, double> rotAxisAngle() const {
        return quaternionToAxisAngle(rotQuat());
    }
};

/*!
 * \brief 回転をオイラー角から初期化
 * \since ver2.5
 * \tparam axis 回転軸の順序
 * \param rot オイラー角 (内的回転の順の3パラメーター)
 *
 */
template <AxisSequence axis>
inline Rotation rotEuler(const std::array<double, 3> &rot) {
    if constexpr (axis == AxisSequence::ZYX) {
        return {rot, std::nullopt};
    } else {
        return {std::nullopt, Rotation::eulerToMatrix(rot, axis)};
    }
}
/*!
 * \brief 回転をオイラー角から初期化
 * \since ver2.5
 *
 * axisを省略した場合 AxisSequence::ZYX
 *
 */
inline Rotation rotEuler(const std::array<double, 3> &rot) {
    return rotEuler<AxisSequence::ZYX>(rot);
}
/*!
 * \brief 回転をオイラー角から初期化
 * \since ver2.5
 * \tparam axis 回転軸の順序 (ver2.5〜)
 *
 */
template <AxisSequence axis>
inline Rotation rotEuler(double angle1, double angle2, double angle3) {
    return rotEuler<axis>(std::array<double, 3>{angle1, angle2, angle3});
}
/*!
 * \brief 回転をオイラー角から初期化
 * \since ver2.5
 *
 * axisを省略した場合 AxisSequence::ZYX
 *
 */
inline Rotation rotEuler(double angle1, double angle2, double angle3) {
    return rotEuler<AxisSequence::ZYX>(
        std::array<double, 3>{angle1, angle2, angle3});
}
/*!
 * \brief 回転を回転行列から初期化
 * \since ver2.5
 * \param pos x, y, z 座標
 * \param rotMatrix 回転行列
 *
 */
inline Rotation rotMatrix(const std::array<std::array<double, 3>, 3> &matrix) {
    return {std::nullopt, matrix};
}
/*!
 * \brief 回転をクォータニオンから初期化
 * \since ver2.5
 * \param quat クォータニオン (w, x, y, z)
 *
 */
inline Rotation rotQuat(const std::array<double, 4> &quat) {
    return rotMatrix(Rotation::quaternionToMatrix(quat));
}
/*!
 * \brief 回転をクォータニオンから初期化
 * \since ver2.5
 */
inline Rotation rotQuat(double w, double x, double y, double z) {
    return rotQuat({w, x, y, z});
}
/*!
 * \brief 回転を回転軸と角度から初期化
 * \since ver2.5
 */
inline Rotation rotAxisAngle(const std::array<double, 3> &axis, double angle) {
    return rotQuat(Rotation::axisAngleToQuaternion(axis, angle));
}

/*!
 * \brief 2次元の回転を初期化
 * \since ver2.5
 * \param rot 回転角(z)
 *
 * rotZ() と同じ。
 *
 */
inline Rotation rot2D(double rot) { return rotEuler(rot, 0, 0); }
/*!
 * \brief X軸周りの回転
 * \since ver2.5
 */
inline Rotation rotX(double rot) { return rotEuler(0, 0, rot); }
/*!
 * \brief Y軸周りの回転
 * \since ver2.5
 */
inline Rotation rotY(double rot) { return rotEuler(0, rot, 0); }
/*!
 * \brief Z軸周りの回転
 * \since ver2.5
 *
 * rot2D() と同じ。
 *
 */
inline Rotation rotZ(double rot) { return rotEuler(rot, 0, 0); }

/*!
 * \brief 3次元の平行移動と回転
 * \since ver1.4
 *
 * \sa Point, Rotation
 */
class Transform : public Point, public Rotation {
  public:
    Transform() : Point(), Rotation() {}
    /*!
     * \since ver2.5
     */
    Transform(const Point &pos, const Rotation &rot)
        : Point(pos), Rotation(rot) {}

    /*!
     * \brief 回転のみの場合Rotationからキャスト
     * \since ver2.5
     */
    Transform(const Rotation &rot) : Point(), Rotation(rot) {}

    /*!
     * \brief オイラー角から初期化
     * \param pos x, y, z 座標
     * \param rot オイラー角
     * 回転軸の順序は z, y, x の順
     * \deprecated ver2.5〜 Transform(pos, rotEuler(rot))
     *
     */
    [[deprecated("use Transform(pos, rotEuler(rot))")]]
    Transform(const Point &pos, const std::array<double, 3> &rot)
        : Transform(pos, Rotation{rot, std::nullopt}) {}

    /*!
     * \brief 2次元の座標と回転を初期化
     * \since ver1.6
     * \param pos x, y 座標
     * \param rot 回転角(z)
     *
     */
    Transform(const Point &pos, double rot) : Transform(pos, rot2D(rot)) {};
    /*!
     * \brief 3次元の座標と回転をオイラー角から初期化
     * \deprecated ver2.5〜 Transform({x, y, z}, rotEuler(z, y, x))
     *
     */
    [[deprecated("use Transform({x, y, z}, rotEuler(z, y, x))")]]
    Transform(double x, double y, double z, double z_angle, double y_angle,
              double x_angle)
        : Transform(std::array<double, 3>{x, y, z},
                    Rotation{std::array<double, 3>{z_angle, y_angle, x_angle},
                             std::nullopt}) {}
    /*!
     * \brief 2次元の座標を初期化
     * \since ver1.6
     * \deprecated ver2.5〜 translate(x, y)
     */
    [[deprecated("use translate(x, y) for translation-only transform")]]
    Transform(double x, double y)
        : Transform({x, y}, 0) {}

    /*!
     * \brief このTransformを別のTransformに適用する
     * \since ver2.5
     *
     * targetを (このTransformの座標系で)
     * 回転&平行移動した結果のTransformを返す。
     * (thisもtargetも変更されない)
     *
     * this * target と同じ。
     *
     */
    WEBCFACE_DLL Transform appliedTo(const Transform &target) const;
    /*!
     * \brief このTransformを別のTransformに適用する
     * \since ver2.5
     *
     * 同次変換行列の積を計算する。
     * 結果はtargetを (このTransformの座標系で)
     * 回転&平行移動した結果 (this->appliedTo(target)) と同じ。
     *
     * \sa appliedTo
     */
    Transform operator*(const Transform &target) const {
        return this->appliedTo(target);
    }
    /*!
     * \brief このTransformをPointに適用する
     * \since ver2.5
     *
     * targetを (このTransformの座標系で)
     * 回転&平行移動した結果のPointを返す。
     * (thisもtargetも変更されない)
     *
     * this * target と同じ。
     *
     */
    WEBCFACE_DLL Point appliedTo(const Point &target) const;
    /*!
     * \brief このTransformをPointに適用する
     * \since ver2.5
     *
     * 同次変換行列とベクトルの積を計算する。
     * 結果はtargetを (このTransformの座標系で)
     * 回転&平行移動した結果 (this->appliedTo(target)) と同じ。
     *
     * \sa appliedTo
     */
    Point operator*(const Point &target) const {
        return this->appliedTo(target);
    }

    bool operator==(const Transform &rhs) const {
        return pos_ == rhs.pos_ && rot() == rhs.rot();
    }
    bool operator!=(const Transform &rhs) const { return !(*this == rhs); }
};

/*!
 * \brief 移動なし、回転なしのTransform
 * \since ver1.4
 */
inline Transform identity() { return {}; }
/*!
 * \brief 平行移動のみのTransform
 * \since ver2.5
 */
inline Transform translate(const Point &pos) { return {pos, Rotation{}}; }
/*!
 * \brief 平行移動のみのTransform
 * \since ver2.5
 */
inline Transform translate(double x, double y, double z) {
    return {{x, y, z}, Rotation{}};
}
/*!
 * \brief 2次元の平行移動のみのTransform
 * \since ver2.5
 */
inline Transform translate(double x, double y) { return {{x, y}, 0}; }

WEBCFACE_NS_END

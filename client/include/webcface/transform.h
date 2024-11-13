#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "array_like.h"
#include <array>
#include <optional>
#include <cassert>

WEBCFACE_NS_BEGIN

class Transform;

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
     * \brief 3次元座標を初期化
     * \since ver2.5
     * std::arrayに限らず任意の配列型(固定長で3要素か、またはvectorのように可変)
     *
     * \todo std::array以外の要素数2の配列の場合エラーになってしまう
     *
     */
    template <typename R,
              typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk,
              typename traits::ArraySizeTrait<R, 3>::SizeMatchOrDynamic =
                  traits::TraitOk>
    Point(const R &pos) : pos_(traits::arrayLikeToArray<3>(pos)) {}

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

    /*!
     * \brief 2つのPointの x, y, z の各要素を加算したPointを返す
     * \since ver2.5
     */
    Point operator+(const Point &other) const {
        return {
            pos_[0] + other.pos_[0],
            pos_[1] + other.pos_[1],
            pos_[2] + other.pos_[2],
        };
    }
    /*!
     * \brief x, y, z の各要素の符号を反転したPointを返す
     * \since ver2.5
     */
    Point operator-() const { return {-pos_[0], -pos_[1], -pos_[2]}; }
    /*!
     * \since ver2.5
     *
     * 何もせずthisを返す
     *
     */
    Point operator+() const { return *this; }
    /*!
     * \since ver2.5
     *
     * this + (-other) と同じ。
     *
     */
    Point operator-(const Point &other) const { return *this + (-other); }
    /*!
     * \since ver2.5
     */
    Point &operator+=(const Point &other) {
        pos_[0] += other.pos_[0];
        pos_[1] += other.pos_[1];
        pos_[2] += other.pos_[2];
        return *this;
    }
    /*!
     * \since ver2.5
     */
    Point &operator-=(const Point &other) { return *this += (-other); }

    /*!
     * \brief x, y, z の各要素をスカラー倍したPointを返す
     * \since ver2.5
     */
    Point operator*(double scalar) const {
        return {pos_[0] * scalar, pos_[1] * scalar, pos_[2] * scalar};
    }
    /*!
     * \brief x, y, z の各要素をスカラー倍したPointを返す
     * \since ver2.5
     */
    friend Point operator*(double scalar, const Point &point) {
        return point * scalar;
    }
    /*!
     * \since ver2.5
     */
    Point &operator*=(double scalar) {
        pos_[0] *= scalar;
        pos_[1] *= scalar;
        pos_[2] *= scalar;
        return *this;
    }
    /*!
     * \brief x, y, z の各要素をスカラーで割ったPointを返す
     * \since ver2.5
     */
    Point operator/(double scalar) const {
        return {pos_[0] / scalar, pos_[1] / scalar, pos_[2] / scalar};
    }
    /*!
     * \since ver2.5
     */
    Point &operator/=(double scalar) {
        pos_[0] /= scalar;
        pos_[1] /= scalar;
        pos_[2] /= scalar;
        return *this;
    }

    /*!
     * * (ver2.5〜) x, y, z の各要素の差が 1e-8 未満のときtrue
     * * (ver2.5〜) Transformとの比較は禁止
     *
     */
    WEBCFACE_DLL bool operator==(const Point &other) const;
    bool operator!=(const Point &other) const { return !(*this == other); }

    bool operator==(const Transform &) = delete;
    bool operator!=(const Transform &) = delete;
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

    friend Rotation rotFromEuler(const std::array<double, 3> &rot,
                                 AxisSequence axis);
    friend Rotation
    rotFromMatrix(const std::array<std::array<double, 3>, 3> &matrix);

    /*!
     * \brief 3次元の回転をオイラー角として取得
     * \sa rotEuler()
     * \deprecated ver2.5〜
     */
    [[deprecated("use rotEuler()")]]
    std::array<double, 3> rot() const {
        return rotEuler();
    }

    /*!
     * \brief 3次元の回転をZYX順のオイラー角として取得
     * \since ver1.6
     * \param index 0→z, 1→y, 2→x
     * \deprecated ver2.5〜
     */
    [[deprecated("use rotEuler()")]]
    double rot(std::size_t index) const {
        return rot().at(index);
    }

    /*!
     * \brief 3次元の回転をオイラー角として取得
     * \since ver2.5
     * \param axis 回転軸の順序 (ver2.5〜)
     *  ver2.4までは z, y, x の順しか指定できない
     *
     */
    std::array<double, 3>
    rotEuler(AxisSequence axis = AxisSequence::ZYX) const {
        if (axis == AxisSequence::ZYX) {
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

    /*!
     * \brief この回転をTransformに適用する
     * \since ver2.5
     *
     * * this * target と同じ。
     * * thisもtargetも変更されない。
     *
     */
    WEBCFACE_DLL Transform appliedTo(const Transform &target) const;
    /*!
     * \brief この回転をTransformに適用する
     * \since ver2.5
     * \sa appliedTo
     */
    WEBCFACE_DLL Transform operator*(const Transform &target) const;
    /*!
     * \brief この回転をRotationに適用する
     * \since ver2.5
     *
     * * this * target と同じ。
     * * thisもtargetも変更されない。
     *
     */
    WEBCFACE_DLL Rotation appliedTo(const Rotation &target) const;
    /*!
     * \brief この回転をRotationに適用する
     * \since ver2.5
     * \sa appliedTo
     */
    WEBCFACE_DLL Rotation operator*(const Rotation &target) const;
    /*!
     * \brief この回転をPointに適用する
     * \since ver2.5
     *
     * * this * target と同じ。
     * * thisもtargetも変更されない。
     *
     */
    WEBCFACE_DLL Point appliedTo(const Point &target) const;
    /*!
     * \brief この回転をPointに適用する
     * \since ver2.5
     * \sa appliedTo
     */
    WEBCFACE_DLL Point operator*(const Point &target) const;

    /*!
     * \brief 逆回転を取得
     * \since ver2.5
     *
     * thisは変更されない。
     *
     */
    WEBCFACE_DLL Rotation inversed() const;

    /*!
     * \brief このRotationを別のRotationに適用した結果で置き換える
     * \since ver2.5
     *
     * this = this * target と同じ。
     *
     */
    Rotation &operator*=(const Rotation &target) {
        *this = *this * target;
        return *this;
    }

    /*!
     * * 回転行列の各要素の差が 1e-8 未満のときtrue
     * * Transformと比較することもできる (平行移動0のTransformとして扱う)
     *
     */
    WEBCFACE_DLL bool operator==(const Rotation &other) const;
    bool operator!=(const Rotation &other) const { return !(*this == other); }

    WEBCFACE_DLL bool operator==(const Transform &other) const;
    bool operator!=(const Transform &other) const { return !(*this == other); }
};

/*!
 * \brief 回転をオイラー角から初期化
 * \since ver2.5
 * \param rot オイラー角 (内的回転の順の3パラメーター)
 * \param axis 回転軸の順序
 *
 */
inline Rotation rotFromEuler(const std::array<double, 3> &rot,
                             AxisSequence axis = AxisSequence::ZYX) {
    if (axis == AxisSequence::ZYX) {
        return {rot, std::nullopt};
    } else {
        return {std::nullopt, Rotation::eulerToMatrix(rot, axis)};
    }
}
/*!
 * \brief 回転をオイラー角から初期化
 * \since ver2.5
 * \param axis 回転軸の順序
 * \param rot オイラー角 (内的回転の順の3パラメーター)
 * std::arrayに限らず任意の配列型(固定長で3要素か、またはvectorのように可変)
 *
 */
template <
    typename R, typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk,
    typename traits::ArraySizeTrait<R, 3>::SizeMatchOrDynamic = traits::TraitOk>
inline Rotation rotFromEuler(const R &rot,
                             AxisSequence axis = AxisSequence::ZYX) {
    return rotFromEuler(traits::arrayLikeToArray<3>(rot), axis);
}
/*!
 * \brief 回転をオイラー角から初期化
 * \since ver2.5
 * \param axis 回転軸の順序
 *
 */
inline Rotation rotFromEuler(double angle1, double angle2, double angle3,
                             AxisSequence axis = AxisSequence::ZYX) {
    return rotFromEuler(std::array<double, 3>{angle1, angle2, angle3}, axis);
}
/*!
 * \brief 回転を回転行列から初期化
 * \since ver2.5
 * \param pos x, y, z 座標
 * \param rotMatrix 回転行列
 *
 * \todo Eigenの配列とか生ポインタとか? 他の型から行列の値を渡せると良い?
 *
 */
inline Rotation
rotFromMatrix(const std::array<std::array<double, 3>, 3> &matrix) {
    return {std::nullopt, matrix};
}
/*!
 * \brief 回転をクォータニオンから初期化
 * \since ver2.5
 * \param quat クォータニオン (w, x, y, z)
 *
 */
inline Rotation rotFromQuat(const std::array<double, 4> &quat) {
    return rotFromMatrix(Rotation::quaternionToMatrix(quat));
}
/*!
 * \brief 回転をクォータニオンから初期化
 * \since ver2.5
 * \param quat クォータニオン (w, x, y, z)
 * std::arrayに限らず任意の配列型(固定長で4要素か、またはvectorのように可変)
 *
 */
template <
    typename R, typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk,
    typename traits::ArraySizeTrait<R, 4>::SizeMatchOrDynamic = traits::TraitOk>
inline Rotation rotFromQuat(const R &quat) {
    return rotFromQuat(traits::arrayLikeToArray<4>(quat));
}
/*!
 * \brief 回転をクォータニオンから初期化
 * \since ver2.5
 */
inline Rotation rotFromQuat(double w, double x, double y, double z) {
    return rotFromQuat({w, x, y, z});
}
/*!
 * \brief 回転を回転軸と角度から初期化
 * \since ver2.5
 */
inline Rotation rotFromAxisAngle(const std::array<double, 3> &axis,
                                 double angle) {
    return rotFromQuat(Rotation::axisAngleToQuaternion(axis, angle));
}
/*!
 * \brief 回転を回転軸と角度から初期化
 * \since ver2.5
 *
 * std::arrayに限らず任意の配列型(固定長で3要素か、またはvectorのように可変)
 *
 */
template <
    typename R, typename traits::ArrayLikeTrait<R>::ArrayLike = traits::TraitOk,
    typename traits::ArraySizeTrait<R, 3>::SizeMatchOrDynamic = traits::TraitOk>
inline Rotation rotFromAxisAngle(const R &axis, double angle) {
    return rotFromAxisAngle(traits::arrayLikeToArray<3>(axis), angle);
}

/*!
 * \brief 2次元の回転を初期化
 * \since ver2.5
 * \param rot 回転角(z)
 *
 * rotZ() と同じ。
 *
 */
inline Rotation rot2D(double rot) { return rotFromEuler(rot, 0, 0); }
/*!
 * \brief X軸周りの回転
 * \since ver2.5
 */
inline Rotation rotX(double rot) { return rotFromEuler(0, 0, rot); }
/*!
 * \brief Y軸周りの回転
 * \since ver2.5
 */
inline Rotation rotY(double rot) { return rotFromEuler(0, rot, 0); }
/*!
 * \brief Z軸周りの回転
 * \since ver2.5
 *
 * rot2D() と同じ。
 *
 */
inline Rotation rotZ(double rot) { return rotFromEuler(rot, 0, 0); }

/*!
 * \brief 3次元の平行移動と回転
 * \since ver1.4
 *
 * * ver2.5〜 publicではなくprivate継承に変更
 *
 * \sa Point, Rotation
 */
class Transform : private Point, private Rotation {
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
    [[deprecated("use translation(x, y) for translation-only transform")]]
    Transform(double x, double y)
        : Transform({x, y}, 0) {}

    friend class Point;
    friend class Rotation;
    using Point::pos;
    using Rotation::rot;
    using Rotation::rotAxisAngle;
    using Rotation::rotEuler;
    using Rotation::rotMatrix;
    using Rotation::rotQuat;

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
     * \brief このTransformをRotationに適用する
     * \since ver2.5
     *
     * targetを (このTransformの座標系で)
     * 回転&平行移動した結果のTransformを返す。
     * 結果はRotationではない。
     * (thisもtargetも変更されない)
     *
     * this * target と同じ。
     *
     */
    Transform appliedTo(const Rotation &target) const {
        return this->appliedTo(Transform(target));
    }
    /*!
     * \brief このTransformをRotationに適用する
     * \since ver2.5
     * \sa appliedTo
     */
    Transform operator*(const Rotation &target) const {
        return this->appliedTo(target);
    }
    /*!
     * \brief このTransformを別のTransformに適用した結果で置き換える
     * \since ver2.5
     *
     * this = this * target と同じ。
     *
     */
    Transform &operator*=(const Transform &target) {
        *this = *this * target;
        return *this;
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

    /*!
     * \brief 逆変換を表すTransformを取得
     * \since ver2.5
     */
    WEBCFACE_DLL Transform inversed() const;

    /*!
     * * (ver2.5〜) x, y, z の各要素と、回転行列の各要素の差が 1e-8
     * 未満のときtrue
     *
     */
    bool operator==(const Transform &other) const {
        return Point(*this) == Point(other) &&
               Rotation(*this) == Rotation(other);
    }
    bool operator!=(const Transform &other) const { return !(*this == other); }

    bool operator==(const Rotation &other) const {
        return *this == Transform(other);
    }
    bool operator!=(const Rotation &other) const { return !(*this == other); }
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
inline Transform translation(const Point &pos) { return {pos, Rotation{}}; }
/*!
 * \brief 平行移動のみのTransform
 * \since ver2.5
 */
inline Transform translation(double x, double y, double z) {
    return {{x, y, z}, Rotation{}};
}
/*!
 * \brief 2次元の平行移動のみのTransform
 * \since ver2.5
 */
inline Transform translation(double x, double y) { return {{x, y}, 0}; }

WEBCFACE_NS_END

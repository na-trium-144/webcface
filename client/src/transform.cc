#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES // NOLINT
#endif
#include "webcface/transform.h"
#include "webcface/exception.h"
#include <cmath>
#include <stdexcept>

WEBCFACE_NS_BEGIN

// https://en.wikipedia.org/wiki/Euler_angles にあるものを写した

std::array<std::array<double, 3>, 3>
Rotation::eulerToMatrix(const std::array<double, 3> &rot, AxisSequence axis) {
    double c0 = std::cos(rot[0]);
    double c1 = std::cos(rot[1]);
    double c2 = std::cos(rot[2]);
    double s0 = std::sin(rot[0]);
    double s1 = std::sin(rot[1]);
    double s2 = std::sin(rot[2]);
    switch (axis) {
    case AxisSequence::XZX:
        return {{
            {{c1, -c2 * s1, s1 * s2}},
            {{c0 * s1, c0 * c1 * c2 - s0 * s2, -c2 * s0 - c0 * c1 * s2}},
            {{s0 * s1, c0 * s2 + c1 * c2 * s0, c0 * c2 - c1 * s0 * s2}},
        }};
    case AxisSequence::XYX:
        return {{
            {{c1, s1 * s2, c2 * s1}},
            {{s0 * s1, c0 * c2 - c1 * s0 * s2, -c0 * s2 - c1 * c2 * s0}},
            {{-c0 * s1, c2 * s0 + c0 * c1 * s2, c0 * c1 * c2 - s0 * s2}},
        }};
    case AxisSequence::YXY:
        return {{
            {{c0 * c2 - c1 * s0 * s2, s0 * s1, c0 * s2 + c1 * c2 * s0}},
            {{s1 * s2, c1, -c2 * s1}},
            {{-c2 * s0 - c0 * c1 * s2, c0 * s1, c0 * c1 * c2 - s0 * s2}},
        }};
    case AxisSequence::YZY:
        return {{
            {{c0 * c1 * c2 - s0 * s2, -c0 * s1, c2 * s0 + c0 * c1 * s2}},
            {{c2 * s1, c1, s1 * s2}},
            {{-c0 * s2 - c1 * c2 * s0, s0 * s1, c0 * c2 - c1 * s0 * s2}},
        }};
    case AxisSequence::ZYZ:
        return {{
            {{c0 * c1 * c2 - s0 * s2, -c2 * s0 - c0 * c1 * s2, c0 * s1}},
            {{c0 * s2 + c1 * c2 * s0, c0 * c2 - c1 * s0 * s2, s0 * s1}},
            {{-c2 * s1, s1 * s2, c1}},
        }};
    case AxisSequence::ZXZ:
        return {{
            {{c0 * c2 - c1 * s0 * s2, -c0 * s2 - c1 * c2 * s0, s0 * s1}},
            {{c2 * s0 + c0 * c1 * s2, c0 * c1 * c2 - s0 * s2, -c0 * s1}},
            {{s1 * s2, c2 * s1, c1}},
        }};
    case AxisSequence::XZY:
        return {{
            {{c1 * c2, -s1, c1 * s2}},
            {{s0 * s2 + c0 * c2 * s1, c0 * c1, c0 * s1 * s2 - c2 * s0}},
            {{c2 * s0 * s1 - c0 * s2, c1 * s0, c0 * c2 + s0 * s1 * s2}},
        }};
    case AxisSequence::XYZ:
        return {{
            {{c1 * c2, -c1 * s2, s1}},
            {{c0 * s2 + c2 * s0 * s1, c0 * c2 - s0 * s1 * s2, -c1 * s0}},
            {{s0 * s2 - c0 * c2 * s1, c2 * s0 + c0 * s1 * s2, c0 * c1}},
        }};
    case AxisSequence::YXZ:
        return {{
            {{c0 * c2 + s0 * s1 * s2, c2 * s0 * s1 - c0 * s2, c1 * s0}},
            {{c1 * s2, c1 * c2, -s1}},
            {{c0 * s1 * s2 - c2 * s0, c0 * c2 * s1 + s0 * s2, c0 * c1}},
        }};
    case AxisSequence::YZX:
        return {{
            {{c0 * c1, s0 * s2 - c0 * c2 * s1, c2 * s0 + c0 * s1 * s2}},
            {{s1, c1 * c2, -c1 * s2}},
            {{-c1 * s0, c0 * s2 + c2 * s0 * s1, c0 * c2 - s0 * s1 * s2}},
        }};
    case AxisSequence::ZYX:
        return {{
            {{c0 * c1, c0 * s1 * s2 - c2 * s0, s0 * s2 + c0 * c2 * s1}},
            {{c1 * s0, c0 * c2 + s0 * s1 * s2, c2 * s0 * s1 - c0 * s2}},
            {{-s1, c1 * s2, c1 * c2}},
        }};
    case AxisSequence::ZXY:
        return {{
            {{c0 * c2 - s0 * s1 * s2, -c1 * s0, c0 * s2 + c2 * s0 * s1}},
            {{c2 * s0 + c0 * s1 * s2, c0 * c1, s0 * s2 - c0 * c2 * s1}},
            {{-c1 * s2, s1, c1 * c2}},
        }};
    default:
        throw InvalidArgument("Invalid axis sequence " +
                              std::to_string(static_cast<int>(axis)));
    }
}

/*
|cos(asin(x)) - 1| がだいたい1ε以下となるxを0とする
sin = x, cos = 1 - x^2 / 2 で近似しちゃうと
x * x / 2 < ε
|x| < sqrt(2ε)
*/
static inline bool isZero(double value) {
    constexpr double sqrt_2_epsilon = 2.1073424255447e-08;
    return value >= -sqrt_2_epsilon && value < sqrt_2_epsilon;
}

// 精度やロバスト性を上げるため、asinやacosは使わないで計算する
// 計算式は使いまわす
// wikipediaの α,β,γ をここでは a,b,c と置いている

static inline std::array<double, 3>
matrixToProperEuler(const std::array<std::array<double, 3>, 3> &rmat,
                    AxisSequence axis) {
    double cb, sa_sb, ca_sb, sc_sb, cc_sb, sc_ca, cc_ca;
    switch (axis) {
    case AxisSequence::XZX:
        cb = rmat[0][0];
        sa_sb = rmat[2][0];
        ca_sb = rmat[1][0];
        sc_sb = rmat[0][2];
        cc_sb = -rmat[0][1];
        sc_ca = rmat[2][1];
        cc_ca = rmat[2][2];
        break;
    case AxisSequence::XYX:
        cb = rmat[0][0];
        sa_sb = rmat[1][0];
        ca_sb = -rmat[2][0];
        sc_sb = rmat[0][1];
        cc_sb = rmat[0][2];
        sc_ca = -rmat[1][2];
        cc_ca = rmat[1][1];
        break;
    case AxisSequence::YXY:
        cb = rmat[1][1];
        sa_sb = rmat[0][1];
        ca_sb = rmat[2][1];
        sc_sb = rmat[1][0];
        cc_sb = -rmat[1][2];
        sc_ca = rmat[0][2];
        cc_ca = rmat[0][0];
        break;
    case AxisSequence::YZY:
        cb = rmat[1][1];
        sa_sb = rmat[2][1];
        ca_sb = -rmat[0][1];
        sc_sb = rmat[1][2];
        cc_sb = rmat[1][0];
        sc_ca = -rmat[2][0];
        cc_ca = rmat[2][2];
        break;
    case AxisSequence::ZYZ:
        cb = rmat[2][2];
        sa_sb = rmat[1][2];
        ca_sb = rmat[0][2];
        sc_sb = rmat[2][1];
        cc_sb = -rmat[2][0];
        sc_ca = rmat[1][0];
        cc_ca = rmat[1][1];
        break;
    case AxisSequence::ZXZ:
        cb = rmat[2][2];
        sa_sb = rmat[0][2];
        ca_sb = -rmat[1][2];
        sc_sb = rmat[2][0];
        cc_sb = rmat[2][1];
        sc_ca = -rmat[0][1];
        cc_ca = rmat[0][0];
        break;
    default:
        throw InvalidArgument("Invalid axis sequence " +
                              std::to_string(static_cast<int>(axis)));
    }
    if ((isZero(sa_sb) && isZero(ca_sb)) || (isZero(sc_sb) && isZero(cc_sb))) {
        return {{
            0,                   // singularity: let sa=0, ca=1
            cb >= 0 ? 0 : -M_PI, // sb = 0, cb = 1,-1
            std::atan2(sc_ca, cc_ca),
        }};
    } else {
        double a = std::atan2(sa_sb, ca_sb);
        return {{
            a,
            std::atan2(sa_sb * std::sin(a) + ca_sb * std::cos(a), cb),
            std::atan2(sc_sb, cc_sb),
        }};
    }
}

static inline std::array<double, 3>
matrixToTaitBryanEuler(const std::array<std::array<double, 3>, 3> &rmat,
                       AxisSequence axis) {
    double sb, sa_cb, ca_cb, sc_cb, cc_cb, sc_ca, cc_ca;
    switch (axis) {
    case AxisSequence::XZY:
        sb = -rmat[0][1];
        sa_cb = rmat[2][1];
        ca_cb = rmat[1][1];
        sc_cb = rmat[0][2];
        cc_cb = rmat[0][0];
        sc_ca = -rmat[2][0];
        cc_ca = rmat[2][2];
        break;
    case AxisSequence::XYZ:
        sb = rmat[0][2];
        sa_cb = -rmat[1][2];
        ca_cb = rmat[2][2];
        sc_cb = -rmat[0][1];
        cc_cb = rmat[0][0];
        sc_ca = rmat[1][0];
        cc_ca = rmat[1][1];
        break;
    case AxisSequence::YXZ:
        sb = -rmat[1][2];
        sa_cb = rmat[0][2];
        ca_cb = rmat[2][2];
        sc_cb = rmat[1][0];
        cc_cb = rmat[1][1];
        sc_ca = -rmat[0][1];
        cc_ca = rmat[0][0];
        break;
    case AxisSequence::YZX:
        sb = rmat[1][0];
        sa_cb = -rmat[2][0];
        ca_cb = rmat[0][0];
        sc_cb = -rmat[1][2];
        cc_cb = rmat[1][1];
        sc_ca = rmat[2][1];
        cc_ca = rmat[2][2];
        break;
    case AxisSequence::ZYX:
        sb = -rmat[2][0];
        sa_cb = rmat[1][0];
        ca_cb = rmat[0][0];
        sc_cb = rmat[2][1];
        cc_cb = rmat[2][2];
        sc_ca = -rmat[1][2];
        cc_ca = rmat[1][1];
        break;
    case AxisSequence::ZXY:
        sb = rmat[2][1];
        sa_cb = -rmat[0][1];
        ca_cb = rmat[1][1];
        sc_cb = -rmat[2][0];
        cc_cb = rmat[2][2];
        sc_ca = rmat[0][2];
        cc_ca = rmat[0][0];
        break;
    default:
        throw InvalidArgument("Invalid axis sequence " +
                              std::to_string(static_cast<int>(axis)));
    }
    if ((isZero(sa_cb) && isZero(ca_cb)) || (isZero(sc_cb) && isZero(cc_cb))) {
        return {{
            0,                              // singularity: let sa=0, ca=1
            sb >= 0 ? M_PI / 2 : -M_PI / 2, // sb = 1,-1  cb = 0
            std::atan2(sc_ca, cc_ca),
        }};
    } else {
        double a = std::atan2(sa_cb, ca_cb);
        return {{
            a,
            std::atan2(sb, sa_cb * std::sin(a) + ca_cb * std::cos(a)),
            std::atan2(sc_cb, cc_cb),
        }};
    }
}

std::array<double, 3>
Rotation::matrixToEuler(const std::array<std::array<double, 3>, 3> &rmat,
                        AxisSequence axis) {
    switch (axis) {
    case AxisSequence::XZX:
    case AxisSequence::XYX:
    case AxisSequence::YXY:
    case AxisSequence::YZY:
    case AxisSequence::ZYZ:
    case AxisSequence::ZXZ:
        return matrixToProperEuler(rmat, axis);
    case AxisSequence::XZY:
    case AxisSequence::XYZ:
    case AxisSequence::YXZ:
    case AxisSequence::YZX:
    case AxisSequence::ZYX:
    case AxisSequence::ZXY:
        return matrixToTaitBryanEuler(rmat, axis);
    default:
        throw InvalidArgument("Invalid axis sequence " +
                              std::to_string(static_cast<int>(axis)));
    }
}

std::array<std::array<double, 3>, 3>
Rotation::quaternionToMatrix(const std::array<double, 4> &quat) {
    double w = quat[0];
    double x = quat[1];
    double y = quat[2];
    double z = quat[3];
    return {{
        {{1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w,
          2 * x * z + 2 * y * w}},
        {{2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z,
          2 * y * z - 2 * x * w}},
        {{2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w,
          1 - 2 * x * x - 2 * y * y}},
    }};
}
std::array<double, 4>
Rotation::matrixToQuaternion(const std::array<std::array<double, 3>, 3> &rmat) {
    double trace = rmat[0][0] + rmat[1][1] + rmat[2][2];
    double w, x, y, z;
    if (trace > 0) {
        double s = std::sqrt(trace + 1.0) * 2;
        w = 0.25 * s;
        x = (rmat[2][1] - rmat[1][2]) / s;
        y = (rmat[0][2] - rmat[2][0]) / s;
        z = (rmat[1][0] - rmat[0][1]) / s;
    } else if (rmat[0][0] > rmat[1][1] && rmat[0][0] > rmat[2][2]) {
        double s = std::sqrt(1.0 + rmat[0][0] - rmat[1][1] - rmat[2][2]) * 2;
        w = (rmat[2][1] - rmat[1][2]) / s;
        x = 0.25 * s;
        y = (rmat[0][1] + rmat[1][0]) / s;
        z = (rmat[0][2] + rmat[2][0]) / s;
    } else if (rmat[1][1] > rmat[2][2]) {
        double s = std::sqrt(1.0 + rmat[1][1] - rmat[0][0] - rmat[2][2]) * 2;
        w = (rmat[0][2] - rmat[2][0]) / s;
        x = (rmat[0][1] + rmat[1][0]) / s;
        y = 0.25 * s;
        z = (rmat[1][2] + rmat[2][1]) / s;
    } else {
        double s = std::sqrt(1.0 + rmat[2][2] - rmat[0][0] - rmat[1][1]) * 2;
        w = (rmat[1][0] - rmat[0][1]) / s;
        x = (rmat[0][2] + rmat[2][0]) / s;
        y = (rmat[1][2] + rmat[2][1]) / s;
        z = 0.25 * s;
    }
    return {w, x, y, z};
}

std::array<double, 4>
Rotation::axisAngleToQuaternion(const std::array<double, 3> &axis,
                                double angle) {
    double half_angle = angle / 2;
    double s = std::sin(half_angle);
    double norm = std::hypot(axis[0], axis[1], axis[2]);
    if (norm == 0) {
        return {1, 0, 0, 0};
    } else {
        return {std::cos(half_angle), axis[0] / norm * s, axis[1] / norm * s,
                axis[2] / norm * s};
    }
}
AxisAngle Rotation::quaternionToAxisAngle(const std::array<double, 4> &quat) {
    double w = quat[0];
    double x = quat[1];
    double y = quat[2];
    double z = quat[3];
    double angle = 2 * std::acos(w);
    return {{x, y, z}, angle};
}

bool Point::operator==(const Point &other) const {
    return std::abs(pos_[0] - other.pos_[0]) < 1e-8 &&
           std::abs(pos_[1] - other.pos_[1]) < 1e-8 &&
           std::abs(pos_[2] - other.pos_[2]) < 1e-8;
}
bool Rotation::operator==(const Rotation &other) const {
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            if (std::abs(rotMatrix(i, j) - other.rotMatrix(i, j)) >= 1e-8) {
                return false;
            }
        }
    }
    return true;
}
bool Rotation::operator==(const Transform &other) const {
    return Transform(*this) == other;
}

Point Rotation::appliedTo(const Point &target) const {
    std::array<double, 3> newPos;
    for (std::size_t i = 0; i < 3; i++) {
        newPos[i] = this->rotMatrix(i, 0) * target.pos(0) +
                    this->rotMatrix(i, 1) * target.pos(1) +
                    this->rotMatrix(i, 2) * target.pos(2);
    }
    return Point{newPos};
}
Rotation Rotation::appliedTo(const Rotation &target) const {
    std::array<std::array<double, 3>, 3> newMatrix;
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            newMatrix[i][j] = this->rotMatrix(i, 0) * target.rotMatrix(0, j) +
                              this->rotMatrix(i, 1) * target.rotMatrix(1, j) +
                              this->rotMatrix(i, 2) * target.rotMatrix(2, j);
        }
    }
    return rotFromMatrix(newMatrix);
}
Transform Rotation::appliedTo(const Transform &target) const {
    return Transform{this->appliedTo(Point{target}),
                     this->appliedTo(Rotation{target})};
}

Point Rotation::operator*(const Point &target) const {
    return appliedTo(target);
}
Rotation Rotation::operator*(const Rotation &target) const {
    return appliedTo(target);
}
Transform Rotation::operator*(const Transform &target) const {
    return appliedTo(target);
}

Rotation Rotation::inversed() const {
    std::array<std::array<double, 3>, 3> newMatrix;
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            newMatrix[i][j] = this->rotMatrix(j, i);
        }
    }
    return rotFromMatrix(newMatrix);
}


Transform Transform::appliedTo(const Transform &target) const {
    return Transform{this->Rotation::appliedTo(target) + Point(*this),
                     this->Rotation::appliedTo(Rotation(target))};
}
Point Transform::appliedTo(const Point &target) const {
    return this->Rotation::appliedTo(target) + Point(*this);
}

Transform Transform::inversed() const {
    std::array<std::array<double, 3>, 3> newMatrix;
    for (std::size_t i = 0; i < 3; i++) {
        for (std::size_t j = 0; j < 3; j++) {
            newMatrix[i][j] = this->rotMatrix(j, i);
        }
    }
    std::array<double, 3> newPos;
    for (std::size_t i = 0; i < 3; i++) {
        newPos[i] = -newMatrix[i][0] * this->pos(0) -
                    newMatrix[i][1] * this->pos(1) -
                    newMatrix[i][2] * this->pos(2);
    }
    return Transform(newPos, rotFromMatrix(newMatrix));
}

WEBCFACE_NS_END

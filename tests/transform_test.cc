#define _USE_MATH_DEFINES // NOLINT
#include <gtest/gtest.h>
#include <random>
#include "webcface/transform.h"
#include <iostream>
#include <cmath>

using namespace webcface;

std::default_random_engine engine{std::random_device()()};
std::uniform_real_distribution dist(-M_PI, M_PI);

TEST(TransformTest, XZX) {
    constexpr auto axis = AxisSequence::XZX;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotX(a)) * Transform(rotZ(b)) * Transform(rotX(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), 0, dist(engine));
    check(dist(engine), M_PI, dist(engine));
    check(dist(engine), -M_PI, dist(engine));
}
TEST(TransformTest, XYX) {
    constexpr auto axis = AxisSequence::XYX;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotX(a)) * Transform(rotY(b)) * Transform(rotX(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), 0, dist(engine));
    check(dist(engine), M_PI, dist(engine));
    check(dist(engine), -M_PI, dist(engine));
}
TEST(TransformTest, YXY) {
    constexpr auto axis = AxisSequence::YXY;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotY(a)) * Transform(rotX(b)) * Transform(rotY(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), 0, dist(engine));
    check(dist(engine), M_PI, dist(engine));
    check(dist(engine), -M_PI, dist(engine));
}
TEST(TransformTest, YZY) {
    constexpr auto axis = AxisSequence::YZY;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotY(a)) * Transform(rotZ(b)) * Transform(rotY(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), 0, dist(engine));
    check(dist(engine), M_PI, dist(engine));
    check(dist(engine), -M_PI, dist(engine));
}
TEST(TransformTest, ZYZ) {
    constexpr auto axis = AxisSequence::ZYZ;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotZ(a)) * Transform(rotY(b)) * Transform(rotZ(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), 0, dist(engine));
    check(dist(engine), M_PI, dist(engine));
    check(dist(engine), -M_PI, dist(engine));
}
TEST(TransformTest, ZXZ) {
    constexpr auto axis = AxisSequence::ZXZ;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotZ(a)) * Transform(rotX(b)) * Transform(rotZ(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), 0, dist(engine));
    check(dist(engine), M_PI, dist(engine));
    check(dist(engine), -M_PI, dist(engine));
}

TEST(TransformTest, XYZ) {
    constexpr auto axis = AxisSequence::XYZ;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotX(a)) * Transform(rotY(b)) * Transform(rotZ(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), M_PI / 2, dist(engine));
    check(dist(engine), -M_PI / 2, dist(engine));
}
TEST(TransformTest, YZX) {
    constexpr auto axis = AxisSequence::YZX;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotY(a)) * Transform(rotZ(b)) * Transform(rotX(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), M_PI / 2, dist(engine));
    check(dist(engine), -M_PI / 2, dist(engine));
}
TEST(TransformTest, ZXY) {
    constexpr auto axis = AxisSequence::ZXY;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotZ(a)) * Transform(rotX(b)) * Transform(rotY(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), M_PI / 2, dist(engine));
    check(dist(engine), -M_PI / 2, dist(engine));
}
TEST(TransformTest, XZY) {
    constexpr auto axis = AxisSequence::XZY;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotX(a)) * Transform(rotZ(b)) * Transform(rotY(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), M_PI / 2, dist(engine));
    check(dist(engine), -M_PI / 2, dist(engine));
}
TEST(TransformTest, ZYX) {
    constexpr auto axis = AxisSequence::ZYX;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotZ(a)) * Transform(rotY(b)) * Transform(rotX(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), M_PI / 2, dist(engine));
    check(dist(engine), -M_PI / 2, dist(engine));
}
TEST(TransformTest, YXZ) {
    constexpr auto axis = AxisSequence::YXZ;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotY(a)) * Transform(rotX(b)) * Transform(rotZ(c));
        Transform tf2 = rotFromEuler(a, b, c, axis);
        Transform tf3 = rotFromEuler(tf2.rotEuler(axis), axis);
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine));
    check(dist(engine), M_PI / 2, dist(engine));
    check(dist(engine), -M_PI / 2, dist(engine));
}

TEST(TransformTest, quat) {
    auto check = [&](double w, double x, double y, double z) {
        double norm = std::sqrt(w * w + x * x + y * y + z * z);
        w /= norm;
        x /= norm;
        y /= norm;
        z /= norm;
        std::cout << "w: " << w << ", x: " << x << ", y: " << y << ", z: " << z
                  << std::endl;
        Transform tf2 = rotFromQuat(w, x, y, z);
        Transform tf3 = rotFromQuat(tf2.rotQuat());
        Transform tf4 = rotFromEuler(tf2.rotEuler());
        auto [axis, angle] = tf2.rotAxisAngle();
        Transform tf5 = rotFromAxisAngle(axis, angle);
        auto axis_angle = tf2.rotAxisAngle();
        Transform tf5_2 =
            rotFromAxisAngle(get<0>(axis_angle), get<1>(axis_angle));
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf2.rotMatrix(i, j), tf3.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf2.rotMatrix(i, j), tf4.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf2.rotMatrix(i, j), tf5.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf2.rotMatrix(i, j), tf5_2.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine), dist(engine));
    check(dist(engine), dist(engine), dist(engine), dist(engine));
}

TEST(TransformTest, inversed) {
    auto check = [&](double a, double b, double c, double x, double y,
                     double z) {
        Transform tf1 = Transform({x, y, z}, rotFromEuler(a, b, c));
        Transform tf2 = tf1 * tf1.inversed();
        Transform tf3 = tf1.inversed() * tf1;
        Transform id = identity();
        for (std::size_t i = 0; i < 3; i++) {
            EXPECT_NEAR(tf2.pos(i), id.pos(i), 1e-8);
            EXPECT_NEAR(tf3.pos(i), id.pos(i), 1e-8);
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_NEAR(tf2.rotMatrix(i, j), id.rotMatrix(i, j), 1e-8);
                EXPECT_NEAR(tf3.rotMatrix(i, j), id.rotMatrix(i, j), 1e-8);
            }
        }
    };
    check(dist(engine), dist(engine), dist(engine), dist(engine), dist(engine),
          dist(engine));
    check(dist(engine), dist(engine), dist(engine), dist(engine), dist(engine),
          dist(engine));
    check(dist(engine), dist(engine), dist(engine), dist(engine), dist(engine),
          dist(engine));
}

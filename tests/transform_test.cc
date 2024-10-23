#include <gtest/gtest.h>
#include <random>
#include "webcface/transform.h"
#include <iostream>

using namespace webcface;

std::default_random_engine engine{std::random_device()()};
std::uniform_real_distribution dist(-M_PI, M_PI);

TEST(TransformTest, XZX) {
    constexpr auto axis = AxisSequence::XZX;
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotX(a)) * Transform(rotZ(b)) * Transform(rotX(c));
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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
        Transform tf2 = rotEuler<axis>(a, b, c);
        Transform tf3 = rotEuler<axis>(tf2.rot<axis>());
        Transform tf4 = rotEuler(tf2.rot());
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

#include <gtest/gtest.h>
#include <random>
#include "webcface/transform.h"
#include <iostream>

using namespace webcface;

std::default_random_engine engine{std::random_device()()};
std::uniform_real_distribution dist(-M_PI, M_PI);

TEST(TransformTest, XZX) {
    auto check = [&](double a, double b, double c) {
        std::cout << "a: " << a << ", b: " << b << ", c: " << c << std::endl;
        Transform tf1 =
            Transform(rotX(a)) * Transform(rotZ(b)) * Transform(rotX(c));
        Transform tf2 = rotEuler<AxisSequence::XZX>(a, b, c);
        Transform tf3 =
            rotEuler<AxisSequence::XZX>(tf2.rot<AxisSequence::XZX>());
        Transform tf4 = rotEuler(tf2.rot());
        for (std::size_t i = 0; i < 3; i++) {
            for (std::size_t j = 0; j < 3; j++) {
                EXPECT_FLOAT_EQ(tf1.rotMatrix(i, j), tf2.rotMatrix(i, j));
                EXPECT_FLOAT_EQ(tf1.rotMatrix(i, j), tf3.rotMatrix(i, j));
                EXPECT_FLOAT_EQ(tf1.rotMatrix(i, j), tf4.rotMatrix(i, j));
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

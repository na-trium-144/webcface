#define _USE_MATH_DEFINES // NOLINT
#include <webcface/client.h>
#include <webcface/canvas3d.h>
#include <chrono>
#include <cmath>

int main() {
    webcface::Client wcli("example_canvas3d");

    {
        using namespace webcface::Geometries;
        using namespace webcface::RobotJoints;
        webcface::RobotLink lp{"plane", plane(webcface::identity(), 10, 10),
                               webcface::ViewColor::gray};
        wcli.robotModel("geometries")
            .set(
                {// plane: 中心座標系と幅、高さを指定 (指定した座標系のxy平面)
                 {"plane", plane(webcface::identity(), 10, 10),
                  webcface::ViewColor::gray},
                 // line: linkの座標系で2点指定
                 {"line1",
                  fixedJoint(
                      "plane",
                      {{0, 0, 0}, webcface::rotFromEuler(0, -M_PI / 2, 0)}),
                  line({0, 0, 0}, {1, 0, 0}), webcface::ViewColor::yellow},
                 {"line2",
                  fixedJoint("line1", {{1, 0, 0},
                                       webcface::rotFromEuler(0, M_PI / 4, 0)}),
                  line({0, 0, 0}, {1, 0, 0}), webcface::ViewColor::green},
                 {"line3", fixedJoint("line1", {0, 0, 0}),
                  line({0.5, 1, 0}, {0.5, -1, 0}), webcface::ViewColor::red},
                 // box: linkの座標系で2点指定
                 {"box", fixedJoint("plane", {1, 0, 0}),
                  box({-0.3, -0.3, 0}, {0.3, 0.3, 0.6}),
                  webcface::ViewColor::yellow},
                 // circle: 中心座標系と半径を指定 (指定した座標系のxy平面)
                 {"circle", fixedJoint("plane", {2, 0, 0}),
                  circle(webcface::translation(0, 0, 0.1), 0.3),
                  webcface::ViewColor::yellow},
                 // cylinder:
                 // 1つの面の中心座標系と半径、押出長さを指定(x正方向に伸びる)
                 {"cylinder", fixedJoint("plane", {3, 0, 0}),
                  cylinder(webcface::rotY(-M_PI / 2), 0.3, 1),
                  webcface::ViewColor::yellow},
                 // sphere: 中心点と半径を指定
                 {"sphere", fixedJoint("plane", {4, 0, 0}),
                  sphere({0, 0, 0.6}, 0.3), webcface::ViewColor::yellow}});

        wcli.robotModel("omniwheel")
            .set({{"base", box({-0.2, -0.2, 0.04}, {0.2, 0.2, 0.06}),
                   webcface::ViewColor::inherit},
                  {"wheel_lf",
                   rotationalJoint("joint_lf", "base",
                                   {{0.2, 0.2, 0.05},
                                    webcface::rotFromEuler(-M_PI / 4, 0, 0)}),
                   cylinder(webcface::rotZ(M_PI / 2), 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"wheel_rf",
                   rotationalJoint("joint_rf", "base",
                                   {{0.2, -0.2, 0.05},
                                    webcface::rotFromEuler(M_PI / 4, 0, 0)}),
                   cylinder(webcface::rotZ(M_PI / 2), 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"wheel_lb",
                   rotationalJoint("joint_lb", "base",
                                   {{-0.2, 0.2, 0.05},
                                    webcface::rotFromEuler(M_PI / 4, 0, 0)}),
                   cylinder(webcface::rotZ(M_PI / 2), 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"wheel_rb",
                   rotationalJoint("joint_rb", "base",
                                   {{-0.2, -0.2, 0.05},
                                    webcface::rotFromEuler(-M_PI / 4, 0, 0)}),
                   cylinder(webcface::rotZ(M_PI / 2), 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"line1", fixedJoint("base", {0, 0, 0.05}),
                   line({0, 0, 0}, {0, 0, 0.3})},
                  {"line2",
                   rotationalJoint("line_rotation", "line1",
                                   webcface::translation(0, 0, 0.3)),
                   line({0, 0, 0}, {0.5, 0, 0}), webcface::ViewColor::red}});
    }

    double i = 0;

    while (true) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
            auto world = wcli.canvas3D("omniwheel_world");
            world.add(
                webcface::plane({}, 3, 3).color(webcface::ViewColor::white));
            world.add(webcface::box({-1.5, -1.5, 0}, {1.5, -1.5, 0.1})
                          .color(webcface::ViewColor::gray));
            world.add(webcface::box({-1.5, 1.5, 0}, {1.5, 1.5, 0.1})
                          .color(webcface::ViewColor::gray));
            world.add(webcface::box({-1.5, -1.5, 0}, {-1.5, 1.5, 0.1})
                          .color(webcface::ViewColor::gray));
            world.add(webcface::box({1.5, -1.5, 0}, {1.5, 1.5, 0.1})
                          .color(webcface::ViewColor::gray));
            world.add(wcli.robotModel("omniwheel")
                          .origin({{-0.3 * std::sin(i / 3.0),
                                    0.3 * std::cos(i / 3.0), 0},
                                   webcface::rotZ(i / 3.0)})
                          .angles({{"line_rotation", -i}}));
            world.sync();
        }
        i += 0.5;

        wcli.loopSyncFor(std::chrono::milliseconds(100));
    }
}

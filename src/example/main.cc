#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
#include <numbers>
#include <cmath>

void hello() { std::cout << "hello, world!" << std::endl; }
double hello2(int a, double b, bool c, const std::string &d) {
    std::cout << "hello2 args: a=" << a << ", b=" << b << ", c=" << c
              << ", d=" << d << std::endl;
    std::cout << "  return: a + b = " << a + b << std::endl;
    return a + b;
}
struct A {
    int x = 1, y = 2;
    A() = default;
    // Dict → A に変換
    A(const webcface::Value::Dict &d) : x(d["x"]), y(d["y"]) {}
    // A → Dictに変換
    operator webcface::Value::Dict() const {
        return {{"x", x}, {"y", y}, {"nest", {{"a", 3}, {"b", 4}}}};
    }
};
int main() {
    webcface::Client c("example_main");

    // c.value("test").set(0);
    c.value("test") = 0;
    // structをDictに変換するとまとめて送信することができる
    c.value("dict") = A();
    // 関数を登録
    c.func("func1") = hello;
    // 引数付きの関数は引数名や各種情報をセットできる
    using Arg = webcface::Arg;
    c.func("func2").set(hello2).setArgs(
        {Arg("a").min(0).max(10), Arg("b"), Arg("c"),
         Arg("d").option({"hoge", "fuga", "piyo"})});

    // 以下のログはすべてwebcfaceに送られる
    c.logger()->trace("this is trace");
    c.logger()->debug("this is debug");
    c.logger()->info("this is info");
    c.logger()->error("this is error");
    c.logger()->critical("this is critical");
    c.logger()->error("Some error message with arg: {}", 1);
    c.loggerOStream() << "this is ostream" << std::endl;
    auto *buf = std::cout.rdbuf();
    std::cout.rdbuf(c.loggerStreamBuf());
    std::cout << "this is cout" << std::endl;
    std::cout.rdbuf(buf);

    {
        using namespace webcface::Geometries;
        using namespace webcface::RobotJoints;
        c.robotModel("geometries")
            .set({// plane: 中心座標系と幅、高さを指定 (指定した座標系のxy平面)
                  {"plane", plane(webcface::identity(), 10, 10),
                   webcface::ViewColor::gray},
                  // line: linkの座標系で2点指定
                  {"line1",
                   fixedJoint("plane",
                              {{0, 0, 0}, {0, -std::numbers::pi / 2, 0}}),
                   line({0, 0, 0}, {1, 0, 0}), webcface::ViewColor::yellow},
                  {"line2",
                   fixedJoint("line1",
                              {{1, 0, 0}, {0, std::numbers::pi / 4, 0}}),
                   line({0, 0, 0}, {1, 0, 0}), webcface::ViewColor::green},
                  {"line3", fixedJoint("line1", {0, 0, 0}),
                   line({0.5, 1, 0}, {0.5, -1, 0}), webcface::ViewColor::red},
                  // box: linkの座標系で2点指定
                  {"box", fixedJoint("plane", {1, 0, 0}),
                   box({-0.3, -0.3, 0}, {0.3, 0.3, 0.6}),
                   webcface::ViewColor::yellow},
                  // circle: 中心座標系と半径を指定 (指定した座標系のxy平面)
                  {"circle", fixedJoint("plane", {2, 0, 0}),
                   circle({0, 0, 0.1, 0, 0, 0}, 0.3),
                   webcface::ViewColor::yellow},
                  // cylinder:
                  // 1つの面の中心座標系と半径、押出長さを指定(x正方向に伸びる)
                  {"cylinder", fixedJoint("plane", {3, 0, 0}),
                   cylinder({0, 0, 0, 0, -std::numbers::pi / 2, 0}, 0.3, 1),
                   webcface::ViewColor::yellow},
                  // sphere: 中心点と半径を指定
                  {"sphere", fixedJoint("plane", {4, 0, 0}),
                   sphere({0, 0, 0.6}, 0.3), webcface::ViewColor::yellow}});

        c.robotModel("omniwheel")
            .set({{"base", box({-0.2, -0.2, 0.04}, {0.2, 0.2, 0.06}),
                   webcface::ViewColor::inherit},
                  {"wheel_lf",
                   rotationalJoint(
                       "joint_lf", "base",
                       {0.2, 0.2, 0.05, -std::numbers::pi / 4, 0, 0}),
                   cylinder({0, 0, 0, std::numbers::pi / 2, 0, 0}, 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"wheel_rf",
                   rotationalJoint(
                       "joint_rf", "base",
                       {0.2, -0.2, 0.05, std::numbers::pi / 4, 0, 0}),
                   cylinder({0, 0, 0, std::numbers::pi / 2, 0, 0}, 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"wheel_lb",
                   rotationalJoint(
                       "joint_lb", "base",
                       {-0.2, 0.2, 0.05, std::numbers::pi / 4, 0, 0}),
                   cylinder({0, 0, 0, std::numbers::pi / 2, 0, 0}, 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"wheel_rb",
                   rotationalJoint(
                       "joint_rb", "base",
                       {-0.2, -0.2, 0.05, -std::numbers::pi / 4, 0, 0}),
                   cylinder({0, 0, 0, std::numbers::pi / 2, 0, 0}, 0.05, 0.01),
                   webcface::ViewColor::inherit},
                  {"line1", fixedJoint("base", {0, 0, 0.05}),
                   line({0, 0, 0}, {0, 0, 0.3})},
                  {"line2",
                   rotationalJoint("line_rotation", "line1",
                                   {0, 0, 0.3, 0, 0, 0}),
                   line({0, 0, 0}, {0.5, 0, 0}), webcface::ViewColor::red}});

        auto cv = c.canvas2D("canvas");
        cv.init(100, 100);
        cv.add(rect({10, 10}, {90, 90}), webcface::ViewColor::black);
        cv.add(line({20, 20}, {80, 80}), webcface::ViewColor::black);
    }

    int i = 0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // valueを更新
        c.value("test") = i;
        // 文字列送信
        c.text("str") = "hello";

        {
            // viewを送信
            auto v = c.view("a");
            v << "hello world" << std::endl;
            v << i << std::endl;
            v << webcface::button("a",
                                  [] { std::cout << "hello" << std::endl; });
            // v.sync();
        }

        auto v = c.view("test_buttons");
        v << "　　" << webcface::button("↑", []{}) << std::endl;
        v << webcface::button("←", []{}) << "　　" << webcface::button("→", []{}) << std::endl;
        v << "　　" << webcface::button("↓", []{}) << std::endl;
        v.sync();

        auto world = c.canvas3D("omniwheel_world");
        world.add(webcface::plane({}, 3, 3), webcface::ViewColor::white);
        world.add(webcface::box({-1.5, -1.5, 0}, {1.5, -1.5, 0.1}),
                  webcface::ViewColor::gray);
        world.add(webcface::box({-1.5, 1.5, 0}, {1.5, 1.5, 0.1}),
                  webcface::ViewColor::gray);
        world.add(webcface::box({-1.5, -1.5, 0}, {-1.5, 1.5, 0.1}),
                  webcface::ViewColor::gray);
        world.add(webcface::box({1.5, -1.5, 0}, {1.5, 1.5, 0.1}),
                  webcface::ViewColor::gray);
        world.add(c.robotModel("omniwheel"),
                  {-0.3 * std::sin(i / 3.0), 0.3 * std::cos(i / 3.0), 0,
                   i / 3.0, 0, 0},
                  {{"line_rotation", -i}});
        world.sync();


        ++i;

        // Dictでまとめて値を取得しstructにセット
        [[maybe_unused]] A a = c.value("dict").getRecurse();

        c.sync();
    }
}

#include <webcface/client.h>
#include <webcface/canvas2d.h>
#include <thread>
#include <iostream>
#include <chrono>
#include <cmath>

int main() {
    webcface::Client wcli("example_canvas2d");

    double i = 0;

    while (true) {
        {
            auto cv = wcli.canvas2D("canvas");
            cv.init(100, 100);
            cv << webcface::rect({10, 10}, {90, 90})
                      .color(webcface::ViewColor::black)
                      .strokeWidth(1);
            cv << webcface::circle(webcface::Point{50, 50}, 10)
                      .color(webcface::ViewColor::black)
                      .fillColor(webcface::ViewColor::gray)
                      .onClick(
                          [] { std::cout << "Canvas Clicked!!" << std::endl; });
            cv << webcface::text("Button")
                      .origin({35, 45})
                      .textColor(webcface::ViewColor::orange)
                      .textSize(10);
            cv << webcface::circle(webcface::Point{50, 50}, 20)
                      .color(webcface::ViewColor::red);
            webcface::Transform pos{
                {50 + 20 * std::cos(i / 3.0), 50 - 20 * std::sin(i / 3.0)},
                -i / 3.0};
            cv << webcface::polygon(
                      {{0, -5}, {-5, 0}, {-5, 10}, {5, 10}, {5, 0}})
                      .origin(pos)
                      .color(webcface::ViewColor::black)
                      .fillColor(webcface::ViewColor::yellow)
                      .strokeWidth(2);
            cv << webcface::circle(webcface::Point{-5, 0}, 2)
                      .origin(pos)
                      .color(webcface::ViewColor::black)
                      .fillColor(webcface::ViewColor::gray)
                      .strokeWidth(0);
            cv << webcface::circle(webcface::Point{-5, 10}, 2)
                      .origin(pos)
                      .color(webcface::ViewColor::black)
                      .fillColor(webcface::ViewColor::gray)
                      .strokeWidth(0);
            cv << webcface::circle(webcface::Point{5, 10}, 2)
                      .origin(pos)
                      .color(webcface::ViewColor::black)
                      .fillColor(webcface::ViewColor::gray)
                      .strokeWidth(0);
            cv << webcface::circle(webcface::Point{5, 0}, 2)
                      .origin(pos)
                      .color(webcface::ViewColor::black)
                      .fillColor(webcface::ViewColor::gray)
                      .strokeWidth(0);
        }

        i += 0.5;

        wcli.waitSyncFor(std::chrono::milliseconds(100));
    }
}

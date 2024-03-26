#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
#include <numbers>
#include <cmath>

int main() {
    webcface::Client wcli("example_main");

    int i = 0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // valueを更新
        wcli.value("test") = i;
        ++i;

        auto v = wcli.view("a");
        v << "hello, world" << std::endl;
        static webcface::InputRef input_val;
        v << webcface::button("cout ",
                              [=] { std::cout << input_val << std::endl; })
          << webcface::textInput("text").bind(input_val) << " => " << input_val
          << std::endl;

        static webcface::InputRef input_num;
        v << webcface::numInput("num").bind(input_num) << " => " << input_num
          << std::endl;

        static webcface::InputRef input_int;
        v << webcface::intInput("int").bind(input_int) << " => " << input_int
          << std::endl;

        v.sync();
        wcli.sync();
    }
}

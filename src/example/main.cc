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
          << webcface::textInput().bind(input_val).init("hello") << " => "
          << input_val << std::endl;

        static webcface::InputRef input_num;
        v << webcface::numInput("num").bind(input_num).min(-15).max(15)
          << " => " << input_num << std::endl;

        static webcface::InputRef input_int;
        v << webcface::intInput("int").bind(input_int).init(5).min(1).max(10)
          << " => " << input_int << std::endl;

        static webcface::InputRef input_select;
        v << webcface::selectInput("select")
                 .bind(input_select)
                 .option({"hoge", "fuga", "piyo"})
          << webcface::toggleInput("toggle")
                 .bind(input_select)
                 .option({"hoge", "fuga", "piyo"})
          << " => " << input_select << std::endl;

        static webcface::InputRef input_slider;
        v << webcface::sliderInput().bind(input_slider).min(0).max(100)
          << " => " << input_slider << std::endl;

        static webcface::InputRef input_check;
        v << webcface::checkInput("check").bind(input_check)
          << " => " << input_check << std::endl;

        v.sync();
        wcli.sync();
    }
}

#include <webcface/client.h>
#include <webcface/view.h>
#include <thread>
#include <iostream>
#include <chrono>
#include <numbers>
#include <cmath>

int main() {
    webcface::Client wcli("example_view");

    int i = 0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        {
            // viewを送信
            auto v = wcli.view("a");
            v << "hello world" << std::endl;
            v << i << std::endl;
            v << webcface::button("a",
                                  [] { std::cout << "hello" << std::endl; });
            v << std::endl;

            webcface::InputRef input_val;
            v << webcface::button("cout ",
                                  [=] { std::cout << input_val << std::endl; })
              << webcface::textInput().bind(input_val).init("hello")
              << std::endl;
            // InputRefはstaticでなくてもよい
            // その代わりここの時点でinput_valの値を表示したりということができない

            static webcface::InputRef input_dec;
            v << webcface::decimalInput("decimal").bind(input_dec).min(-15).max(
                     15)
              << " => " << input_dec << std::endl;

            static webcface::InputRef input_num;
            v << webcface::numberInput("number")
                     .bind(input_num)
                     .init(5)
                     .min(2)
                     .max(10)
                     .step(2)
              << " => " << input_num << std::endl;

            static webcface::InputRef input_select, input_toggle;
            v << webcface::selectInput("select")
                     .bind(input_select)
                     .option({"hoge", "fuga", "piyo"})
              << " => " << input_select << std::endl;

            v << webcface::toggleInput("toggle")
                     .bind(input_toggle)
                     .option({"hoge", "fuga", "piyo"})
              << " => " << input_toggle << std::endl;

            static webcface::InputRef input_slider;
            v << webcface::sliderInput()
                     .bind(input_slider)
                     .min(0)
                     .max(100)
                     .step(10)
              << " => " << input_slider << std::endl;

            static webcface::InputRef input_check;
            v << webcface::checkInput("check").bind(input_check) << " => "
              << input_check << std::endl;

            // v.sync();
        }

        {
            auto v = wcli.view("test_buttons");
            v << "　　" << webcface::button("↑", [] {}) << std::endl;
            v << webcface::button("←", [] {}) << "　　"
              << webcface::button("→", [] {}) << std::endl;
            v << "　　" << webcface::button("↓", [] {}) << std::endl;
            v.sync();
        }
        i++;
        wcli.sync();
    }
}

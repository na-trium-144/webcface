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
        webcface::InputRef input_val;
        v << webcface::button("aaaa", [=] { std::cout << input_val << std::endl; })
          << webcface::input().bind(input_val);

        v.sync();
        wcli.sync();
    }
}

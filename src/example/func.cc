#include <chrono>
#include <cmath>
#include <iostream>
#include <numbers>
#include <thread>
#include <webcface/webcface.h>

void hello() { std::cout << "hello, world!" << std::endl; }
double hello2(int a, double b, bool c, const std::string &d) {
    std::cout << "hello2 args: a=" << a << ", b=" << b << ", c=" << c
              << ", d=" << d << std::endl;
    std::cout << "  return: a + b = " << a + b << std::endl;
    return a + b;
}

int main() {
    webcface::Client wcli("example_func");

    // 関数を登録
    wcli.func("func1") = hello;

    wcli.func("lambda") = [](const std::string &str) {
        std::cout << "lambda(" << str << ")" << std::endl;
    };

    // 引数付きの関数は引数名や各種情報をセットできる
    using Arg = webcface::Arg;
    wcli.func("func2").set(hello2).setArgs(
        {Arg("a").min(0).max(10), Arg("b"), Arg("c"),
         Arg("d").option({"hoge", "fuga", "piyo"})});

    wcli.func("func_bool").set([](bool) -> bool { return true; });
    wcli.func("func_int").set([](int) -> int { return 1; });
    wcli.func("func_double").set([](double) -> double { return 1.5; });
    wcli.func("func_str").set([](std::string) -> std::string { return "1"; });

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        wcli.sync();
    }
}

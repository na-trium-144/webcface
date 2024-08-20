#include <webcface/client.h>
#include <webcface/func.h>
#include <chrono>
#include <iostream>
#include <thread>

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
    wcli.func("func1").set(hello);

    wcli.func("lambda").set([](const std::string &str) {
        std::cout << "lambda(" << str << ")" << std::endl;
    });

    // 引数付きの関数は引数名や各種情報をセットできる
    using Arg = webcface::Arg;
    wcli.func("func2").set(hello2).setArgs(
        {Arg("a").min(0).max(10), Arg("b"), Arg("c"),
         Arg("d").option({"hoge", "fuga", "piyo"})});

    wcli.func("func_bool").set([](bool) -> bool { return true; });
    wcli.func("func_int").set([](int) -> int { return 1; });
    wcli.func("func_double").set([](double) -> double { return 1.5; });
    wcli.func("func_str").set([](const std::string &) -> std::string {
        return "1";
    });

    // 引数型が非対応の場合:
    // ↓ error: no type named ArgTypesSupportedByWebCFaceFunc
    //           in webcface::FuncArgTypesTrait<std::nullptr_t, int>
    // wcli.func("nyan").set([](std::nullptr_t, int) {});

    wcli.loopSync();
}

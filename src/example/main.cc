#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
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
    A(const webcface::Value::Dict &d): x(d["x"]), y(d["y"]) {}
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
        ++i;

        // Dictでまとめて値を取得しstructにセット
        A a = c.value("dict").getRecurse();

        c.sync();
    }
}

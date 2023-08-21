#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
void hello() { std::cout << "hello, world!" << std::endl; }
double hello2(int a, double b, bool c, std::string d) {
    std::cout << "hello, world! 2" << std::endl;
    std::cout << "  a: " << a << std::endl;
    std::cout << "  b: " << b << std::endl;
    std::cout << "  c: " << c << std::endl;
    std::cout << "  d: " << d << std::endl;
    return a + b;
}
int main() {
    WebCFace::Client c("example_main");
    c.value("test") = 0;
    c.func("func1") = hello;
    using Arg = WebCFace::Arg;
    c.func("func2").set(hello2).setArgs(
        {Arg("a").min(0).max(10), Arg("b"), Arg("c"),
         Arg("d").option({"hoge", "fuga", "piyo"})});
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        c.value("test")++;
        c.text("str") = "hello";
        std::cout << "send str = " << c.text("str") << std::endl;
        std::cout << "send test = " << c.value("test") << std::endl;
        std::cout << "recv str = " << c.member("example_main").text("str")
                  << std::endl;
        std::cout << "recv test = " << c.member("example_main").value("test")
                  << std::endl;

        // c.func("aaaaaaa").run(); // self().func("aaaaaa") is not set
        // c.member("aaaaaa").func("a").run(); // member("aaaaaa").func("a") is
        // not set
        c.func("func1").run();
        // c.func("func2").run(3, true, "hoge"); // invalid_argument
        // c.member("example_main").func("func2").run(3, true, "hoge");
        // runtime_error
        auto f = c.member("example_main")
                     .func("func2")
                     .runAsync(3, 5.5, true, "hoge");
        // // ValAdaptor -> int, double, bool, string etc...
        std::cout << "return = " << static_cast<double>(f.result.get())
                  << std::endl;

        c.sync();
    }
}
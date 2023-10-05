#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
void hello() { std::cout << "hello, world!" << std::endl; }
double hello2(int a, double b, bool c, const std::string &d) {
    std::cout << "hello, world! 2" << std::endl;
    std::cout << "  a: " << a << std::endl;
    std::cout << "  b: " << b << std::endl;
    std::cout << "  c: " << c << std::endl;
    std::cout << "  d: " << d << std::endl;
    return a + b;
}
int main() {
    WebCFace::logger_internal_level = spdlog::level::trace;

    WebCFace::Client c("example_main");
    c.value("test") = 0;
    c.value("dict") = {{"x", 1}, {"y", 2}, {"nest", {{"a", 3}, {"b", 4}}}};
    c.func("func1") = hello;
    using Arg = WebCFace::Arg;
    c.func("func2").set(hello2).setArgs(
        {Arg("a").min(0).max(10), Arg("b"), Arg("c"),
         Arg("d").option({"hoge", "fuga", "piyo"})});

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
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        c.value("test")++;
        c.text("str") = "hello";

        {
            auto v = c.view("a");
            v << "hello world" << std::endl;
            v << i << WebCFace::newLine();
            v << WebCFace::button("a",
                                  [] { std::cout << "hello" << std::endl; });
            // v.sync();
        }
        ++i;
        
        // c.func("aaaaaaa").run(); // self().func("aaaaaa") is not set
        // c.member("aaaaaa").func("a").run(); // member("aaaaaa").func("a") is
        // not set
        // c.func("func1").run();
        // c.func("func2").run(3, true, "hoge"); // invalid_argument
        // c.member("example_main").func("func2").run(3, true, "hoge");
        // runtime_error
        // auto f = c.member("example_main")
        //              .func("func2")
        //              .runAsync(3, 5.5, true, "hoge");
        // // ValAdaptor -> int, double, bool, string etc...
        // std::cout << "return = " << static_cast<double>(f.result.get())
        //           << std::endl;
        c.sync();
    }
}
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
    c.self().value("test") = 0;
    c.self().func("func1") = hello;
    c.self().func("func2") = hello2;
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        c.self().value("test")++;
        c.self().text("str") = "hello";
        std::cout << "send str = " << c.self().text("str") << std::endl;
        std::cout << "send test = " << c.self().value("test") << std::endl;
        std::cout << "recv str = " << c.member("example_main").text("str")
                  << std::endl;
        std::cout << "recv test = " << c.member("example_main").value("test")
                  << std::endl;

        // c.func("aaaaaaa").run(); bad_function_call
        c.self().func("func1").run(1);
        // c.func("func2").run(3, true, "hoge"); invalid_argument
        auto f = c.self().func("func2").run(3, 5.5, true, "hoge");
        // ValAdaptor -> int, double, bool, string etc...
        std::cout << "return = " << static_cast<int>(f.get()) << std::endl;

        c.send();
    }
}
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
    WebCFace::Client c("example_simple");
    c.value("test") = 0;
    c.func("hello") = hello;
    c.func("hello2") = hello2;
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        c.value("test") += 1;
        c.sync();
    }
}
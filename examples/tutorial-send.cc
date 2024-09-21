#include <iostream>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/func.h>

webcface::Client wcli("tutorial-send");

int hoge() {
    std::cout << "Function hoge started" << std::endl;
    return 42;
}
int fuga(int a, const std::string &b) {
    std::cout << "Function fuga(" << a << ", " << b << ") started" << std::endl;
    return a;
}

int main() {
    wcli.func("hoge").set(hoge);
    wcli.func("fuga").set(fuga);
    wcli.waitConnection();

    std::cout << "Hello, World! (sender)" << std::endl;

    wcli.value("data") = 100;
    wcli.text("message") = "Hello, World! (sender)";
    wcli.sync();

    std::cout << "sender finish" << std::endl;
}

#include <iostream>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>

webcface::Client wcli("tutorial-send");

int main() {
    wcli.waitConnection();

    std::cout << "Hello, World! (sender)" << std::endl;

    wcli.value("data") = 42;
    wcli.text("message") = "Hello, World! (sender)";
    wcli.sync();
    
    std::cout << "sender finish" << std::endl;
}

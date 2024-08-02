#include <iostream>
#include <webcface/webcface.h>

int main() {
    webcface::Client wcli("tutorial");
    wcli.start();

    std::cout << "Hello, World!" << std::endl;
}

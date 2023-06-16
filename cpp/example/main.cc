#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
int main() {
    WebCFace::Client c("example_main");
    c.value("test") = 0;
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        c.value("test") = c.value("test") + 1;
        c.value("test") += 1;
        --c.value("test");
        std::cout << "send test = " << c.value("test") << std::endl;
        std::cout << "recv test = " << c.value("example_main", "test")
                  << std::endl;
        c.send();
    }
}
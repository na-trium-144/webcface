#include <webcface/webcface.h>
#include <thread>
#include <chrono>
#include <iostream>
int main() {
    WebCFace::Client c("example_get_entry");
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for (const auto &s : c.subjects()) {
            std::cout << s.name() << std::endl;
            auto e = s.entry();
            for (const auto &v : e.value) {
                std::cout << "  value " << v << std::endl;
            }
            for (const auto &t : e.text) {
                std::cout << "  text " << t << std::endl;
            }
        }
        std::cout << std::endl;
        c.send();
    }
}
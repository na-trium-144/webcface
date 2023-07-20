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
            for (const auto &v : s.values()) {
                std::cout << "  value " << v.name << std::endl;
            }
            for (const auto &t : s.texts()) {
                std::cout << "  text " << t.name << std::endl;
            }
        }
        std::cout << std::endl;
        c.send();
    }
}
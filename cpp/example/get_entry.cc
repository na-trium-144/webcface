#include <webcface/webcface.h>
#include <thread>
#include <chrono>
#include <iostream>
int main() {
    WebCFace::Client c("example_get_entry");
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        for (auto &m : c.members()) {
            std::cout << m.name() << std::endl;
            for (auto &v : m.values()) {
                std::cout << "  value " << v.name() << std::endl;
            }
            for (auto &t : m.texts()) {
                std::cout << "  text " << t.name() << std::endl;
            }
            for (auto &f : m.funcs()) {
                std::cout << "  func " << f.name() << " arg: ";
                auto args = f.argsType();
                for (std::size_t i = 0; i < args.size(); i++) {
                    if (i > 0) {
                        std::cout << ", ";
                    }
                    std::cout << args[i];
                }
                std::cout << " ret = " << f.returnType() << std::endl;
            }
        }
        std::cout << std::endl;
        c.send();
    }
}
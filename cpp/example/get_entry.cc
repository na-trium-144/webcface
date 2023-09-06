#include <webcface/webcface.h>
#include <thread>
#include <chrono>
#include <iostream>
int main() {
    WebCFace::Client c("example_get_entry");
    c.onMemberEntry().appendListener([](WebCFace::Member m) {
        std::cout << "member " << m.name() << std::endl;
        m.onValueEntry().appendListener([](WebCFace::Value v) {
            std::cout << "value " << v.name() << std::endl;
        });
        m.onTextEntry().appendListener([](WebCFace::Text v) {
            std::cout << "text " << v.name() << std::endl;
        });
        m.onFuncEntry().appendListener([](WebCFace::Func f) {
            std::cout << "func " << f.name() << " arg: ";
            auto args = f.args();
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    std::cout << ", ";
                }
                std::cout << args[i];
            }
            std::cout << " ret: " << f.returnType() << std::endl;
        });
        m.log().appendListener([](WebCFace::LogLine l) {
            std::cout << "log [" << l.level << "] " << l.message << std::endl;
        });
    });
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        /*for (auto &m : c.members()) {
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
        std::cout << std::endl;*/
        c.sync();
    }
}
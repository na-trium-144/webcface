#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
int main() {
    WebCFace::logger_internal_level = spdlog::level::trace;
    WebCFace::Client c("example_recv");
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
        m.log().appendListener([](WebCFace::Log l) {
            for (const auto &ll : l.get()) {
                std::cout << "log [" << ll.level << "] " << ll.message
                          << std::endl;
            }
            // l.clear();
        });
    });
    while (true) {
        std::this_thread::yield();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        std::cout << "recv test = " << c.member("example_main").value("test")
                  << std::endl;

        // c.value("example_main", "test") += 2;
        // -> error: candidate function template not viable: ... method is not
        // marked const
        std::thread([&] {
            try {
                auto result = c.member("example_main")
                                  .func("func2")
                                  .run(9, 7.1, false, "");
                std::cout << "func2(9, 7.1, false, \"\") = "
                          << static_cast<std::string>(result) << std::endl;
            } catch (...) {
            }
        }).detach();
        c.sync();
    }
}
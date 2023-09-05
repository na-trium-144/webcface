#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
int main() {
    WebCFace::logger_internal_level = spdlog::level::trace;
    WebCFace::Client c("example_recv");
    c.member("example_main")
        .value("test")
        .appendListener([](const WebCFace::Value &v) {
            std::cout << "ValueChangeEvent test = " << v << std::endl;
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
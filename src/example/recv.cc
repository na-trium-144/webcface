#include <webcface/webcface.h>
#include <thread>
#include <iostream>
#include <chrono>
int main() {
    // webcface::logger_internal_level = spdlog::level::trace;
    webcface::Client c("example_recv");
    c.onMemberEntry().appendListener([](webcface::Member m) {
        std::cout << "member entry " << m.name() << std::endl;
        m.onValueEntry().appendListener([](webcface::Value v) {
            std::cout << "value entry " << v.name() << std::endl;
        });
        m.onTextEntry().appendListener([](webcface::Text v) {
            std::cout << "text entry " << v.name() << std::endl;
        });
        m.onFuncEntry().appendListener([](webcface::Func f) {
            std::cout << "func entry " << f.name() << " arg: ";
            auto args = f.args();
            for (std::size_t i = 0; i < args.size(); i++) {
                if (i > 0) {
                    std::cout << ", ";
                }
                std::cout << args[i];
            }
            std::cout << " ret: " << f.returnType() << std::endl;
        });
        m.log().appendListener([](webcface::Log l) {
            for (const auto &ll : l.get()) {
                std::cout << "log [" << ll.level << "] " << ll.message
                          << std::endl;
            }
            l.clear();
        });
    });
    c.start();
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // example_mainのtestの値を取得する
        std::cout << "example_main.test = "
                  << c.member("example_main").value("test") << std::endl;
        // example_mainのfunc1を実行する
        c.member("example_main").func("func1").runAsync();
        // example_mainのfunc2を実行し結果を取得
        auto result =
            c.member("example_main").func("func2").runAsync(9, 7.1, false, "");
        try {
            std::cout << "func2(9, 7.1, false, \"\") = "
                      << static_cast<std::string>(result.result.get())
                      << std::endl;
        } catch (...) {
        }
    }
}
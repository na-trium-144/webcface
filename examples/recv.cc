#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/func.h>
#include <webcface/log.h>
#include <thread>
#include <iostream>
#include <chrono>
int main() {
    // webcface::logger_internal_level = spdlog::level::trace;
    webcface::Client c("example_recv");
    c.onMemberEntry([](const webcface::Member &m) {
        std::cout << "member entry " << m.name() << std::endl;
        m.onValueEntry([](const webcface::Value &v) {
            std::cout << "value entry " << v.name() << std::endl;
        });
        m.onTextEntry([](const webcface::Text &v) {
            std::cout << "text entry " << v.name() << std::endl;
        });
        m.onFuncEntry([](const webcface::Func &f) {
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
        m.log().onChange([](const webcface::Log &l) {
            for (const auto &ll : l.get()) {
                std::cout << "log [" << ll.level() << "] " << ll.message()
                          << std::endl;
            }
            l.clear();
        });
    });
    c.start();
    auto func_m = c.member("example_func");
    while (true) {
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        // example_mainのtestの値を取得する
        std::cout << "example_main.test = "
                  << c.member("example_value").value("test") << std::endl;

        // example_mainのfunc1を実行する
        func_m.func("func1").runAsync();
        // example_mainのfunc2を実行し結果を取得
        auto result = func_m.func("func2").runAsync(9, 7.1, false, "");
        result.onFinish([](const webcface::Promise &result) {
            std::cout << "func2(9, 7.1, false, \"\") = "
                      << result.response().asStringView() << std::endl;
        });

        func_m.func("func_bool").runAsync(true);
        func_m.func("func_int").runAsync(1);
        func_m.func("func_double").runAsync(1.0);
        func_m.func("func_str").runAsync("1");
        c.loopSyncFor(std::chrono::milliseconds(100));
    }
}
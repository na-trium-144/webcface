#include <webcface/client.h>
#include <webcface/log.h>
#include <thread>
#include <iostream>
#include <chrono>

int main() {
    webcface::Client wcli("example_log");

    // 以下のログはすべてwebcfaceに送られる
    wcli.log().append(webcface::level::trace, "this is trace");
    wcli.log().append(webcface::level::debug, "this is debug");
    wcli.log().append(webcface::level::info, "this is info");
    wcli.log().append(webcface::level::error, "this is error");
    wcli.log().append(webcface::level::critical, "this is critical");
    // wcli.log().append(webcface::level::error,
    //                   std::format("Some error message with arg: {}", 1));

    // ostreamを使う場合
    wcli.loggerOStream() << "this is ostream" << std::endl;

    // coutを置き換える場合
    auto *buf = std::cout.rdbuf();
    std::cout.rdbuf(wcli.loggerStreamBuf());
    std::cout << "this is cout" << std::endl;
    std::cout.rdbuf(buf);

    int i = 0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        wcli.loggerOStream() << "info " << i++ << std::endl;

        wcli.sync();
    }
}

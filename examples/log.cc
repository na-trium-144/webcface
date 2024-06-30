#include <webcface/client.h>
#include <webcface/logger.h>
#include <thread>
#include <iostream>
#include <chrono>
#include <numbers>
#include <cmath>

int main() {
    webcface::Client wcli("example_log");

    // 以下のログはすべてwebcfaceに送られる
    wcli.logger()->trace("this is trace");
    wcli.logger()->debug("this is debug");
    wcli.logger()->info("this is info");
    wcli.logger()->error("this is error");
    wcli.logger()->critical("this is critical");
    wcli.logger()->error("Some error message with arg: {}", 1);

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
        wcli.logger()->info("info {}", i++);

        wcli.sync();
    }
}

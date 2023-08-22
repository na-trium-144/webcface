#include <webcface/webcface.h>
#include <thread>
#include <chrono>
#include <iostream>
int main() {
    WebCFace::Client c("example_logger");
    c.logger()->set_level(spdlog::level::trace);
    c.logger()->trace("this is trace");
    c.logger()->debug("this is debug");
    c.logger()->info("this is info");
    c.logger()->error("this is error");
    c.logger()->error("Some error message with arg: {}", 1);
    c.logger_ostream() << "this is ostream" << std::endl;
    auto *buf = std::cout.rdbuf();
    std::cout.rdbuf(c.logger_streambuf());
    std::cout << "this is cout" << std::endl;
    std::cout.rdbuf(buf);
    while(true){
        std::this_thread::yield();
    }
}
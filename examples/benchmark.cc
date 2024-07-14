#include <webcface/client.h>
#include <webcface/text.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <optional>

int main() {
    webcface::Client wcli1("bench1"), wcli2;
    wcli1.waitConnection();
    wcli2.waitConnection();
    auto bench1_recv = wcli2.member(wcli1.name());
    std::chrono::steady_clock::time_point start_t;
    std::optional<std::chrono::steady_clock::time_point> recv_t;
    bench1_recv.text("a").appendListener([&](auto) {
        std::cout << "recv" << std::endl;
        recv_t = std::chrono::steady_clock::now();
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    for (int s = 10; s <= 1000000; s *= 10) {
        std::cout << "message size = " << s << " bytes" << std::endl;
        int latency_total = 0;
        for (int i = 0; i < 10; i++) {
            start_t = std::chrono::steady_clock::now();
            recv_t = std::nullopt;
            wcli1.text("a") = std::string(s, static_cast<char>('a' + i));
            wcli1.sync();
            do {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } while (!recv_t);
            int latency = static_cast<int>(
                std::chrono::duration_cast<std::chrono::microseconds>(*recv_t -
                                                                      start_t)
                    .count());
            std::cout << "latency: " << latency << " us" << std::endl;
            latency_total += latency;
        }
        std::cout << "average latency: " << latency_total / 10.0 << " us"
                  << std::endl;
        std::cout << std::endl;
    }
}

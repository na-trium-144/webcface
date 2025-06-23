#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/server/server.h>
#include <string>
#include <vector>
#include <atomic>

static void LatencyMultipleValue(benchmark::State &state) {
    webcface::server::Server server(27530, 0);
    webcface::Client wcli1("bench1", "127.0.0.1", 27530),
        wcli2("bench2", "127.0.0.1", 27530);
    wcli1.waitConnection();
    wcli2.waitConnection();
    auto bench1_recv = wcli2.member(wcli1.name());
    std::vector<std::string> names(state.range(0));
    std::atomic<int> recv_c;
    for (int i = 0; i < state.range(0); i++) {
        names[i] = std::string("test_") + std::to_string(i);
        bench1_recv.value(names[i]).onChange([&](const auto &) { recv_c++; });
    }
    int v = 0;
    for (auto _ : state) {
        recv_c.store(0);
        for (const std::string &n : names) {
            wcli1.value(n) = ++v;
        }
        wcli1.sync();
        do {
            wcli2.sync();
        } while (recv_c.load() < state.range(0));
    }
}
BENCHMARK(LatencyMultipleValue)->Range(1, 1024);

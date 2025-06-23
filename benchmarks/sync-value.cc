#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/server/server.h>
#include <string>
#include <vector>

static void SyncMultipleValue(benchmark::State &state) {
    webcface::server::Server server(27530, 0);
    webcface::Client wcli1("bench1");
    wcli1.waitConnection();
    std::vector<std::string> names(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
        names[i] = std::string("test_") + std::to_string(i);
    }
    int v = 0;
    for (auto _ : state) {
        for (const std::string &n : names) {
            wcli1.value(n) = ++v;
        }
        wcli1.sync();
    }
}
BENCHMARK(SyncMultipleValue)->Range(1, 1024);

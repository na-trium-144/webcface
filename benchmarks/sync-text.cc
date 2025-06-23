#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/text.h>
#include <webcface/server/server.h>

static void SyncLongText(benchmark::State &state) {
    server::Server server(27530, 0);
    webcface::Client wcli1("bench1");
    wcli1.waitConnection();
    unsigned char c = ' ';
    for (auto _ : state) {
        wcli1.text("test") = std::string(state.range(0), c);
        c = (c - 32 + 1) % (256 - 32) + 32;
        wcli1.sync();
    }
}
BENCHMARK(SyncLongText)->Range(1, 1048576);

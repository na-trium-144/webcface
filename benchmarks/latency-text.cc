#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/text.h>
#include <webcface/server/server.h>
#include <atomic>

static void SyncLongText(benchmark::State &state) {
    webcface::server::Server server(27530, 0);
    webcface::Client wcli1("bench1"), wcli2("bench2");
    wcli1.waitConnection();
    wcli2.waitConnection();
    auto bench1_recv = wcli2.member(wcli1.name());
    std::atomic<int> recv_c;
    bench1_recv.text("test").onChange([&](const auto &) { recv_c.store(1); });
    unsigned char c = ' ';
    for (auto _ : state) {
        recv_c.store(0);
        wcli1.text("test") = std::string(state.range(0), c);
        c = (c - 32 + 1) % (256 - 32) + 32;
        wcli1.sync();
        do {
            wcli2.sync();
        } while (recv_c.load() < state.range(0));
    }
}
BENCHMARK(SyncLongText)->Range(1, 1048576);

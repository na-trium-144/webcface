#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/text.h>
#include <webcface/server/server.h>
#include <atomic>
#include <iostream>
#include <memory>

static std::unique_ptr<webcface::server::Server> server;
static std::unique_ptr<webcface::Client> wcli1, wcli2;
static std::atomic<int> recv_c;
static unsigned char c = ' ';
static void DoSetup(const benchmark::State &){
    // std::cerr << "-- LatencyLongText " << state.range(0) << std::endl;
    server = std::make_unique<webcface::server::Server>(27530, 4);
    wcli1 = std::make_unique<webcface::Client>("bench1", "127.0.0.1", 27530);
    wcli2 = std::make_unique<webcface::Client>("bench2", "127.0.0.1", 27530);
    wcli1->waitConnection();
    wcli2->waitConnection();
    wcli2->member(wcli1->name()).text("test").onChange([&](const auto &) { recv_c.store(1); });
}
static void DoTeardown(const benchmark::State &){
    wcli1.reset();
    wcli2.reset();
    server.reset();
}
static void LatencyLongText(benchmark::State &state) {
    for (auto _ : state) {
        recv_c.store(0);
        wcli1->text("test") = std::string(state.range(0), c);
        c = (c - 32 + 1) % (127 - 32) + 32;
        wcli1->sync();
        do {
            wcli2->sync();
        } while (recv_c.load() < state.range(0));
    }
}
BENCHMARK(LatencyLongText)->RangeMultiplier(16)->Range(1, 1048576)->Setup(DoSetup)->Teardown(DoTeardown);

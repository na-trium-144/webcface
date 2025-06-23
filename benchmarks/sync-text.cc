#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/text.h>
#include <webcface/server/server.h>
#include <iostream>

static std::unique_ptr<webcface::server::Server> server;
static std::unique_ptr<webcface::Client> wcli1;
static unsigned char c = ' ';
static void DoSetup(const benchmark::State &){
    // std::cerr << "-- SyncLongText " << state.range(0) << std::endl;
    server = std::make_unique<webcface::server::Server>(27530, 4);
    wcli1 = std::make_unique<webcface::Client>("bench1", "127.0.0.1", 27530);
    wcli1->waitConnection();
}
static void DoTeardown(const benchmark::State &){
    wcli1.reset();
    server.reset();
}
static void SyncLongText(benchmark::State &state) {
    for (auto _ : state) {
        wcli1->text("test") = std::string(state.range(0), c);
        c = (c - 32 + 1) % (127 - 32) + 32;
        wcli1->sync();
    }
}
BENCHMARK(SyncLongText)->RangeMultiplier(16)->Range(1, 1048576)->Setup(DoSetup)->Teardown(DoTeardown);

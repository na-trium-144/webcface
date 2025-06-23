#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/server/server.h>
#include <string>
#include <vector>
#include <iostream>

static std::unique_ptr<webcface::server::Server> server;
static std::unique_ptr<webcface::Client> wcli1;
static std::vector<std::string> names;
static void DoSetup(const benchmark::State &state){
    // std::cerr << "-- SyncMultipleValue " << state.range(0) << std::endl;
    server = std::make_unique<webcface::server::Server>(27530, 4);
    wcli1 = std::make_unique<webcface::Client>("bench1", "127.0.0.1", 27530);
    wcli1->waitConnection();
    names.resize(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
        names[i] = std::string("test_") + std::to_string(i);
    }
}
static void DoTeardown(const benchmark::State &){
    wcli1.reset();
    server.reset();
}

static int v = 0;
static void SyncMultipleValue(benchmark::State &state) {
    for (auto _ : state) {
        for (const std::string &n : names) {
            wcli1->value(n) = ++v;
        }
        wcli1->sync();
    }
}
BENCHMARK(SyncMultipleValue)->RangeMultiplier(4)->Range(1, 1024)->Setup(DoSetup)->Teardown(DoTeardown);

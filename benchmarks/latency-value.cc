#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/value.h>
#include "../src/server/store.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string>
#include <vector>
#include <atomic>
#include <iostream>

static std::unique_ptr<webcface::Server::Server> server;
static std::unique_ptr<webcface::Client> wcli1, wcli2;
// static std::unique_ptr<std::thread> sync_thread;
static std::atomic<int> recv_c;
static std::vector<std::string> names;
static int p = 37530;
static void DoSetup(const benchmark::State &state){
    // std::cerr << "-- LatencyMultipleValue " << state.range(0) << std::endl;
    auto stderr_sink =
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    stderr_sink->set_level(spdlog::level::err);
    server = std::make_unique<webcface::Server::Server>(p, stderr_sink,
                                                  spdlog::level::trace);
    wcli1 = std::make_unique<webcface::Client>("bench1", "127.0.0.1", p);
    wcli2 = std::make_unique<webcface::Client>("bench2", "127.0.0.1", p);
    p++;
    wcli1->waitConnection();
    wcli2->waitConnection();
    // sync_thread = std::make_unique<std::thread>([&]{wcli2->loopSync();});
    names.resize(state.range(0));
    for (int i = 0; i < state.range(0); i++) {
        names[i] = std::string("test_") + std::to_string(i);
        wcli2->member(wcli1->name()).value(names[i]).appendListener([&](const auto &) { recv_c++; });
    }
}
static void DoTeardown(const benchmark::State &){
    wcli1.reset();
    wcli2->close();
    // sync_thread->join();
    // sync_thread.reset();
    wcli2.reset();
    server.reset();
}
static int v = 0;
static void LatencyMultipleValue(benchmark::State &state) {
    for (auto _ : state) {
        recv_c.store(0);
        for (const std::string &n : names) {
            wcli1->value(n) = ++v;
        }
        wcli1->sync();
        do {
        } while (recv_c.load() < state.range(0));
    }
}
BENCHMARK(LatencyMultipleValue)->RangeMultiplier(4)->Range(1, 1024)->Setup(DoSetup)->Teardown(DoTeardown);

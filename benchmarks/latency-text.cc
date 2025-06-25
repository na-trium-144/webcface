#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/text.h>
#include "../src/server/store.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <atomic>
#include <iostream>
#include <memory>

static std::unique_ptr<webcface::Server::Server> server;
static std::unique_ptr<webcface::Client> wcli1, wcli2;
// static std::unique_ptr<std::thread> sync_thread;
static std::atomic<int> recv_c;
static unsigned char c = ' ';
static int p = 27530;
static void DoSetup(const benchmark::State &){
    // std::cerr << "-- LatencyLongText " << state.range(0) << std::endl;
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
    wcli2->member(wcli1->name()).text("test").appendListener([&](const auto &) { recv_c.store(1); });
}
static void DoTeardown(const benchmark::State &){
    wcli1.reset();
    wcli2->close();
    // sync_thread->join();
    // sync_thread.reset();
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
        } while (recv_c.load() == 0);
    }
}
BENCHMARK(LatencyLongText)->RangeMultiplier(16)->Range(1, 1048576)->Setup(DoSetup)->Teardown(DoTeardown);

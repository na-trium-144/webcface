#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/server/server.h>
#include <string>
#include <vector>
#include <atomic>
#include <iostream>

namespace BM_LatencyMultipleValue {
    std::unique_ptr<webcface::server::Server> server;
    std::unique_ptr<webcface::Client> wcli1, wcli2;
    std::unique_ptr<std::thread> sync_thread;
    std::atomic<int> recv_c;
    std::vector<std::string> names;
    void DoSetup(const benchmark::State &state){
        // std::cerr << "-- LatencyMultipleValue " << state.range(0) << std::endl;
        server = std::make_unique<webcface::server::Server>(27530, 4);
        wcli1 = std::make_unique<webcface::Client>("bench1", "127.0.0.1", 27530);
        wcli2 = std::make_unique<webcface::Client>("bench2", "127.0.0.1", 27530);
        wcli1->waitConnection();
        wcli2->waitConnection();
        sync_thread = std::make_unique<std::thread>([&]{wcli2->loopSync();});
        names.resize(static_cast<std::size_t>(state.range(0)));
        for (int i = 0; i < state.range(0); i++) {
            names[i] = std::string("test_") + std::to_string(i);
            wcli2->member(wcli1->name()).value(names[i]).onChange([&](const auto &) { recv_c++; });
        }
    }
    void DoTeardown(const benchmark::State &){
        wcli1.reset();
        wcli2->close();
        sync_thread->join();
        sync_thread.reset();
        wcli2.reset();
        server.reset();
    }
    int v = 0;
}
void LatencyMultipleValue(benchmark::State &state) {
    using namespace BM_LatencyMultipleValue;
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
BENCHMARK(LatencyMultipleValue)->RangeMultiplier(4)->Range(1, 1024)->Setup(BM_LatencyMultipleValue::DoSetup)->Teardown(BM_LatencyMultipleValue::DoTeardown);

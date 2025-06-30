#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/text.h>
#include <webcface/server/server.h>
#include <atomic>
#include <iostream>
#include <memory>

namespace BM_LatencyLongText {
    std::unique_ptr<webcface::server::Server> server;
    std::unique_ptr<webcface::Client> wcli1, wcli2;
    std::unique_ptr<std::thread> sync_thread;
    std::atomic<int> recv_c;
    unsigned char c = ' ';
    void DoSetup(const benchmark::State &){
        // std::cerr << "-- LatencyLongText " << state.range(0) << std::endl;
        server = std::make_unique<webcface::server::Server>(27530, 4);
        wcli1 = std::make_unique<webcface::Client>("bench1", "127.0.0.1", 27530);
        wcli2 = std::make_unique<webcface::Client>("bench2", "127.0.0.1", 27530);
        wcli1->waitConnection();
        wcli2->waitConnection();
        sync_thread = std::make_unique<std::thread>([&]{wcli2->loopSync();});
        wcli2->member(wcli1->name()).text("test").onChange([&](const auto &) { recv_c.store(1); });
    }
    void DoTeardown(const benchmark::State &){
        wcli1.reset();
        wcli2->close();
        sync_thread->join();
        sync_thread.reset();
        wcli2.reset();
        server.reset();
    }
}
void LatencyLongText(benchmark::State &state) {
    using namespace BM_LatencyLongText;
    for (auto _ : state) {
        recv_c.store(0);
        wcli1->text("test") = std::string(static_cast<std::size_t>(state.range(0)), c);
        c = (c - 32 + 1) % (127 - 32) + 32;
        wcli1->sync();
        do {
        } while (recv_c.load() == 0);
    }
}
BENCHMARK(LatencyLongText)->RangeMultiplier(16)->Range(1, 1048576)->Setup(BM_LatencyLongText::DoSetup)->Teardown(BM_LatencyLongText::DoTeardown);

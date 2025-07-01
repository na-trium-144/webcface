#include <benchmark/benchmark.h>
#include <webcface/client.h>
#include <webcface/text.h>
#include <webcface/server/server.h>
#include <iostream>

namespace BM_SyncLongText{
    // std::unique_ptr<webcface::server::Server> server;
    std::unique_ptr<webcface::Client> wcli1;
    // std::unique_ptr<std::thread> sync_thread;
    unsigned char c = ' ';
    void DoSetup(const benchmark::State &){
        // std::cerr << "-- SyncLongText " << state.range(0) << std::endl;
        // server = std::make_unique<webcface::server::Server>(27530, 4);
        wcli1 = std::make_unique<webcface::Client>("bench1", "127.0.0.1", 27530);
        // wcli2 = std::make_unique<webcface::Client>("bench2", "127.0.0.1", 27530);
        // wcli1->waitConnection();
        // wcli2->waitConnection();
        // sync_thread = std::make_unique<std::thread>([&]{wcli2->loopSync();});
    }
    void DoTeardown(const benchmark::State &){
        wcli1.reset();
        // wcli2->close();
        // sync_thread->join();
        // sync_thread.reset();
        // wcli2.reset();
        // server.reset();
    }
}
void SyncLongText(benchmark::State &state) {
    using namespace BM_SyncLongText;
    for (auto _ : state) {
        std::string str(static_cast<std::size_t>(state.range(0)), c);
        wcli1->text("test") = str;
        c = (c - 32 + 1) % (127 - 32) + 32;
        wcli1->sync();
        // state.PauseTiming();
        // do{
        // }while(wcli2->member(wcli1->name()).text("test").get() != str);
        // state.ResumeTiming();
    }
}
BENCHMARK(SyncLongText)->RangeMultiplier(16)->Range(1, 1048576)->Setup(BM_SyncLongText::DoSetup)->Teardown(BM_SyncLongText::DoTeardown);

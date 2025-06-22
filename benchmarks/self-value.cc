#include <webcface/client.h>
#include <webcface/value.h>
// #include <webcface/server/server.h>
#include <benchmark/benchmark.h>

static void BM_SelfValue(benchmark::State& state) {
  // server::Server server(27530, 0);
  // wcli1.waitConnection();
  webcface::Client wcli1("bench1");
  int i = 0;
  for (auto _ : state) {
    wcli1.value("a") = ++i;
    wcli1.sync();
  }
}
BENCHMARK(BM_SelfValue);

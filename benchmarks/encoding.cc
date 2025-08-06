#include <benchmark/benchmark.h>
#include <webcface/common/encoding.h>

void stringConstruct(benchmark::State &state){
    std::string str = "hello, world";
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::string(str));
    }
}
void SharedStringConstruct(benchmark::State &state){
    std::string str = "hello, world";
    for (auto _ : state) {
        benchmark::DoNotOptimize(webcface::SharedString::fromU8String(str));
    }
}
void stringConstructStatic(benchmark::State &state){
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::string("hello, world"));
    }
}
void SharedStringConstructStatic(benchmark::State &state){
    for (auto _ : state) {
        benchmark::DoNotOptimize(webcface::SharedString::fromU8StringStatic("hello, world"));
    }
}
void stringView(benchmark::State &state){
    std::string str = "hello, world";
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::string_view(str));
    }
}
void SharedStringView(benchmark::State &state){
    webcface::SharedString ss = webcface::SharedString::fromU8StringStatic("hello, world");
    for (auto _ : state) {
        benchmark::DoNotOptimize(ss.u8StringView());
    }
}
void stringCompare(benchmark::State &state){
    std::string str1 = "hello, world";
    std::string str2 = "hello, world2";
    for (auto _ : state) {
        benchmark::DoNotOptimize(str1 == str2);
    }
}
void SharedStringCompare(benchmark::State &state){
    webcface::SharedString ss1 = webcface::SharedString::fromU8StringStatic("hello, world");
    webcface::SharedString ss2 = webcface::SharedString::fromU8StringStatic("hello, world2");
    for (auto _ : state) {
        benchmark::DoNotOptimize(ss1 == ss2);
    }
}

BENCHMARK(stringConstruct);
BENCHMARK(SharedStringConstruct);
BENCHMARK(stringConstructStatic);
BENCHMARK(SharedStringConstructStatic);
BENCHMARK(stringView);
BENCHMARK(SharedStringView);
BENCHMARK(stringCompare);
BENCHMARK(SharedStringCompare);

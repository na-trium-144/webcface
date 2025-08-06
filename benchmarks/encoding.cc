#include <benchmark/benchmark.h>
#include <webcface/common/encoding.h>

void stringConstruct(benchmark::State &state){
    std::vector<std::string> strings;
    for (int i = 0; i < 10; i++){
        strings.emplace_back("hello, world " + std::to_string(i));
    }
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::string(strings[i++ % 10]));
    }
}
void SharedStringConstruct(benchmark::State &state){
    std::vector<std::string> strings;
    for (int i = 0; i < 10; i++){
        strings.emplace_back("hello, world " + std::to_string(i));
    }
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(webcface::SharedString::fromU8String(strings[i++ % 10]));
    }
}
void stringConstructStatic(benchmark::State &state){
    const char strings[][15] = {
        "hello, world 0", "hello, world 1", "hello, world 2", "hello, world 3",
        "hello, world 4", "hello, world 5", "hello, world 6", "hello, world 7",
        "hello, world 8", "hello, world 9"
    };
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(std::string(strings[i++ % 10]));
    }
}
void SharedStringConstructStatic(benchmark::State &state){
    const char strings[][15] = {
        "hello, world 0", "hello, world 1", "hello, world 2", "hello, world 3",
        "hello, world 4", "hello, world 5", "hello, world 6", "hello, world 7",
        "hello, world 8", "hello, world 9"
    };
    int i = 0;
    for (auto _ : state) {
        benchmark::DoNotOptimize(webcface::SharedString::fromU8StringStatic(
            std::string_view(strings[i++ % 10])));
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
void SharedStringViewShare(benchmark::State &state){
    webcface::SharedString ss = webcface::SharedString::fromU8StringStatic("hello, world");
    for (auto _ : state) {
        benchmark::DoNotOptimize(ss.u8StringViewShare());
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
BENCHMARK(SharedStringViewShare);
BENCHMARK(stringCompare);
BENCHMARK(SharedStringCompare);

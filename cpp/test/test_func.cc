#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include "utils.h"

DROGON_TEST(FuncTest) {
    WebCFace::Client cli1("test1"), cli2("test2");
    wait();
    cli1.sync();
    cli2.sync();
    int called = 0;
    auto m1 = cli1.self();
    auto m2 = cli2.member("test1");
    m1.func("a") = [&called] { ++called; };
    m1.func("b") = [&called](int a, double b, bool c, const std::string &d) {
        return a + b * 10 + c + d.size();
    };
    m1.func("a").run();
    CHECK(called == 1);
    m2.func("a").run();
    CHECK(called == 2);
    auto res1 = m1.func("a").runAsync();
    auto res2 = m2.func("a").runAsync();
    res1.result.get();
    res2.result.get();
    CHECK(called == 4);

    CHECK(static_cast<double>(m1.func("b").run(1, 2.5, true, "aaaaa")) == 32);
    CHECK(static_cast<double>(m2.func("b").run(1, 2.5, true, "aaaaa")) == 32);
    CHECK(static_cast<double>(
              m1.func("b").runAsync(1, 2.5, true, "aaaaa").result.get()) == 32);
    CHECK(static_cast<double>(
              m2.func("b").runAsync(1, 2.5, true, "aaaaa").result.get()) == 32);
}
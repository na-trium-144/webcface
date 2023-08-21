#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include "utils.h"

struct FuncTestScopeGuard {
    inline static std::mutex mtx;
    FuncTestScopeGuard() { mtx.lock(); }
    ~FuncTestScopeGuard() { mtx.unlock(); }
};

DROGON_TEST(FuncTest) {
    WebCFace::Client cli1("test1"), cli2("test2");
    wait();
    cli1.sync();
    cli2.sync();
    int called = 0;
    WebCFace::Member m1 = cli1;
    auto m2 = cli2.member("test1");
    auto f1a = m1.func("a") = [&called] { ++called; };
    auto f2a = m2.func("a");
    cli1.setDefaultRunCondOnSync();
    auto f1d = m1.func("d") = [&called] { ++called; };
    cli1.setDefaultRunCondNone();
    auto f1b =
        m1.func("b") = [](int a, double b, bool c, const std::string &d) {
            return a + b * 10 + c + d.size();
        };
    auto f2b = m2.func("b");
    f1a.run();
    CHECK(called == 1);
    f2a.run();
    CHECK(called == 2);
    auto res1 = f1a.runAsync();
    auto res2 = f2a.runAsync();
    res1.result.get();
    res2.result.get();
    CHECK(called == 4);

    f1a.setRunCondOnSync().runAsync();
    f2a.runAsync();
    wait();
    CHECK(called == 4);
    cli1.sync();
    CHECK(called == 6);

    {
        FuncTestScopeGuard guard;
        f1a.setRunCondScopeGuard<FuncTestScopeGuard>().runAsync();
        f2a.runAsync();
        wait();
        CHECK(called == 6);
    }
    wait();
    CHECK(called == 8);

    f1d.setRunCondOnSync().runAsync();
    wait();
    CHECK(called == 8);
    cli1.sync();
    CHECK(called == 9);

    CHECK(static_cast<double>(f1b.run(1, 2.5, true, "aaaaa")) == 32);
    CHECK(static_cast<double>(f2b.run(1, 2.5, true, "aaaaa")) == 32);
    CHECK(static_cast<double>(
              f1b.runAsync(1, 2.5, true, "aaaaa").result.get()) == 32);
    CHECK(static_cast<double>(
              f2b.runAsync(1, 2.5, true, "aaaaa").result.get()) == 32);

    CHECK(m1.func("c").runAsync().started.get() == false);
    CHECK(m2.func("c").runAsync().started.get() == false);
    CHECK(cli2.member("hoge").func("c").runAsync().started.get() == false);
}
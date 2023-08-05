#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include "utils.h"

DROGON_TEST(ValueTest) {
    WebCFace::Client cli1("test1"), cli2("test2");
    const int v = 1;
    const std::string t = "aaa";
    wait();
    cli1.self().value("v") = v;
    cli1.self().text("t") = t;
    bool cli1_ev = false;
    cli1.sync();
    cli2.sync();
    wait();
    CHECK(cli1.self().value("v") == v);
    CHECK(cli1.self().text("t") == t);
    // 1回目は無
    CHECK(cli1.member("test1").value("v").tryGet() == std::nullopt);
    CHECK(cli1.member("test1").value("v") == 0);
    CHECK(cli2.member("test1").value("v").tryGet() == std::nullopt);
    CHECK(cli2.member("test1").value("v") == 0);
    CHECK(cli1.member("test1").text("t").tryGet() == std::nullopt);
    CHECK(cli1.member("test1").text("t") == "");
    CHECK(cli2.member("test1").text("t").tryGet() == std::nullopt);
    CHECK(cli2.member("test1").text("t") == "");

    CHECK(cli2.members().size() == 1);
    CHECK(cli2.members()[0].values().size() == 1);
    CHECK(cli2.members()[0].texts().size() == 1);
    cli1.sync();
    cli2.sync();
    wait();
    CHECK(cli1.member("test1").value("v") == v);
    CHECK(cli2.member("test1").value("v") == v);
    CHECK(cli1.member("test1").text("t") == t);
    CHECK(cli2.member("test1").text("t") == t);
}
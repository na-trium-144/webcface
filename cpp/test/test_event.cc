#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include "utils.h"

DROGON_TEST(EventTest) {
    WebCFace::Client cli1("test1"), cli2("test2");
    wait();
    cli1.value("v") = 1;
    cli1.text("t") = "aaa";
    cli1.func("f") = [](int a, double b, bool c, const std::string &d) {
        return 100;
    };
    cli1.func("f").setArgs({
        WebCFace::Arg("a"),
        WebCFace::Arg("b").min(-100).max(100),
        WebCFace::Arg("c").init(false),
        WebCFace::Arg("d").option({"hoge", "fuga", "piyo"}),
    });
    int callback_called = 0;
    cli2.memberEntry().appendListener([TEST_CTX, &callback_called](auto m) {
        std::cout << "memberEntry" << std::endl;
        CHECK(m.name() == "test1");
        callback_called++;
    });
    cli2.member("test1").valueEntry().appendListener(
        [TEST_CTX, &callback_called](auto v) {
            std::cout << "valueEntry" << std::endl;
            CHECK(v.name() == "v");
            callback_called++;
        });
    cli2.member("test1").textEntry().appendListener(
        [TEST_CTX, &callback_called](auto t) {
            std::cout << "textEntry" << std::endl;
            CHECK(t.name() == "t");
            callback_called++;
        });
    cli2.member("test1").funcEntry().appendListener(
        [TEST_CTX, &callback_called](auto f) {
            std::cout << "funcEntry" << std::endl;
            CHECK(f.name() == "f");
            CHECK(f.args()[0].name() == "a");
            CHECK(f.args()[0].type() == WebCFace::ValType::int_);
            CHECK(f.args()[1].name() == "b");
            CHECK(f.args()[1].type() == WebCFace::ValType::float_);
            CHECK(*f.args()[1].min() == -100);
            CHECK(*f.args()[1].max() == 100);
            CHECK(f.args()[2].name() == "c");
            CHECK(f.args()[2].type() == WebCFace::ValType::bool_);
            CHECK(static_cast<bool>(*f.args()[2].init()) == false);
            CHECK(f.args()[3].name() == "d");
            CHECK(f.args()[3].type() == WebCFace::ValType::string_);
            CHECK(std::string(f.args()[3].option()[0]) == "hoge");
            CHECK(std::string(f.args()[3].option()[1]) == "fuga");
            CHECK(std::string(f.args()[3].option()[2]) == "piyo");
            CHECK(f.returnType() == WebCFace::ValType::int_);
            callback_called++;
        });
    cli2.member("test1").value("v").appendListener(
        [TEST_CTX, &callback_called](auto v) {
            std::cout << "valueChange" << std::endl;
            CHECK(v == 1);
            callback_called++;
        });
    cli2.member("test1").text("t").appendListener(
        [TEST_CTX, &callback_called](auto t) {
            std::cout << "textChange" << std::endl;
            CHECK(t == "aaa");
            callback_called++;
        });
    cli1.sync();
    cli2.sync();
    wait();
    CHECK(callback_called == 6);
}

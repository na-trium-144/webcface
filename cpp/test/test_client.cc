#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include <chrono>
#include <thread>
#include <future>
#include "../server/store.h"

void wait() { std::this_thread::sleep_for(std::chrono::milliseconds(200)); }
DROGON_TEST(ConnectionTest) {
    WebCFace::Client cli1("test1");
    wait();
    CHECK(cli1.connected());

    // cli1.close();
    // wait();
    // CHECK(!cli1.connected());
}
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
DROGON_TEST(EventTest) {
    WebCFace::Client cli1("test1"), cli2("test2");
    wait();
    cli1.self().value("v") = 1;
    cli1.self().text("t") = "aaa";
    cli1.self().func("f") = [](int a, double b, bool c, const std::string &d) {
        return 100;
    };
    cli1.self().func("f").setArgs({
        WebCFace::Arg("a"),
        WebCFace::Arg("b").min(-100).max(100),
        WebCFace::Arg("c").init(false),
        WebCFace::Arg("d").option({"hoge", "fuga", "piyo"}),
    });
    int callback_called = 0;
    cli2.membersChange().appendListener([TEST_CTX, &callback_called](auto m) {
        std::cout << "membersChange" << std::endl;
        CHECK(m.name() == "test1");
        callback_called++;
    });
    cli2.member("test1").valuesChange().appendListener(
        [TEST_CTX, &callback_called](auto v) {
            std::cout << "valuesChange" << std::endl;
            CHECK(v.name() == "v");
            callback_called++;
        });
    cli2.member("test1").textsChange().appendListener(
        [TEST_CTX, &callback_called](auto t) {
            std::cout << "textsChange" << std::endl;
            CHECK(t.name() == "t");
            callback_called++;
        });
    cli2.member("test1").funcsChange().appendListener(
        [TEST_CTX, &callback_called](auto f) {
            std::cout << "funcsChange" << std::endl;
            CHECK(f.name() == "f");
            CHECK(f.args()[0].name() == "a");
            CHECK(f.args()[0].type() == WebCFace::ValType::int_);
            CHECK(f.args()[1].name() == "b");
            CHECK(f.args()[1].type() == WebCFace::ValType::float_);
            CHECK(f.args()[1].min() == -100);
            CHECK(f.args()[1].max() == 100);
            CHECK(f.args()[2].name() == "c");
            CHECK(f.args()[2].type() == WebCFace::ValType::bool_);
            CHECK(f.args()[2].init() == false);
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

int main(int argc, char **argv) {
    using namespace drogon;
    WebCFace::Server::controllerKeeper();

    app().addListener("0.0.0.0", 80);

    std::promise<void> p1;
    std::future<void> f1 = p1.get_future();

    app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });

    // Start the main loop on another thread
    WebCFace::Client a{};
    // std::thread thr([]() {
    //     // Queues the promise to be fulfilled after starting the loop
    //     app().run();
    // });

    // The future is only satisfied after the event loop started
    f1.get();

    int status = test::run(argc, argv);

    // Ask the event loop to shutdown and wait
    // app().getLoop()->queueInLoop([]() { app().quit(); });
    // thr.join();
    return status;
}

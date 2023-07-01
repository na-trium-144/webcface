#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include <chrono>
#include <thread>

DROGON_TEST(ClientTest)
{
    WebCFace::Client cli1("test1"), cli2("test2");
    const int v = 1;
    const std::string t = "aaa";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    cli1.value("v") = v;
    cli1.text("t") = t;
    cli1.send();
    cli2.send();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK(cli1.value("v") == v);
    CHECK(cli1.text("t") == t);
    // 1回目は無
    CHECK(cli1.value("test1", "v").try_get() == std::nullopt);
    CHECK(cli1.value("test1", "v") == 0);
    CHECK(cli2.value("test1", "v").try_get() == std::nullopt);
    CHECK(cli2.value("test1", "v") == 0);
    CHECK(cli1.text("test1", "t").try_get() == std::nullopt);
    CHECK(cli1.text("test1", "t") == "");
    CHECK(cli2.text("test1", "t").try_get() == std::nullopt);
    CHECK(cli2.text("test1", "t") == "");
    cli1.send();
    cli2.send();
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    CHECK(cli1.value("test1", "v") == v);
    CHECK(cli2.value("test1", "v") == v);
    CHECK(cli1.text("test1", "t") == t);
    CHECK(cli2.text("test1", "t") == t);
}

int main(int argc, char** argv) 
{
    using namespace drogon;
    
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

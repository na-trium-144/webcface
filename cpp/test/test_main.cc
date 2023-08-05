#define DROGON_TEST_MAIN
#include <drogon/drogon_test.h>
#include <drogon/drogon.h>
#include <webcface/webcface.h>
#include <future>
#include "../server/store.h"
#include "utils.h"

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

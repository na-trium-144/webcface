#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>

namespace WebCFace {

void init() {
    using namespace drogon;
    static auto conn =
        WebSocketClient::newWebSocketClient("localhost", 80, false);
    static auto req = HttpRequest::newHttpRequest();
    req->setPath("/");

    conn->setMessageHandler(
        [](const std::string &message, const WebSocketClientPtr &ws,
           const WebSocketMessageType &type) { std::cout << "recv msg\n"; });
    conn->setConnectionClosedHandler(
        [](const WebSocketClientPtr &ws) { std::cout << "closed\n"; });

    conn->connectToServer(req, [](ReqResult r, const HttpResponsePtr &resp,
                                  const WebSocketClientPtr &ws) {
        auto c = ws->getConnection();
        if (r == ReqResult::Ok) {
            std::cout << "connected\n";
        } else {
            std::cout << "error\n";
        }
    });

    static struct MainLoop {
        std::optional<std::thread> thr;
        MainLoop() {
            std::promise<void> p1;
            std::future<void> f1 = p1.get_future();

            // Start the main loop on another thread
            thr = std::make_optional<std::thread>([&]() {
                // Queues the promise to be fulfilled after starting the loop
                app().getLoop()->queueInLoop([&p1]() { p1.set_value(); });
                app().run();
            });

            // The future is only satisfied after the event loop started
            f1.get();
        }
        ~MainLoop() {
            app().getLoop()->queueInLoop([]() { app().quit(); });
            thr->join();
        }
    } q;
}
} // namespace WebCFace
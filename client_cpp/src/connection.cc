#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>
#include <string>
#include "entry.h"
#include "../../server_cpp/message/message.h"
// todo: これインストールしたら動かないのでmessage.hの場所を考え直す必要がある

namespace WebCFace {

void init(const std::string& name, const std::string& host, int port) {
    entry.setName(name);

    using namespace drogon;
    static auto conn =
        WebSocketClient::newWebSocketClient(host, port, false);
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
            // todo: エラー時どうするか
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
#include "connection.h"
#include <drogon/WebSocketClient.h>
#include <drogon/HttpAppFramework.h>
#include <future>
#include <optional>
#include <string>
#include "entry.h"
#include "../../server_cpp/message/message.h"
// todo: message.hの場所を考え直す必要があるか?

namespace WebCFace {
drogon::WebSocketClientPtr ws;

void init(const std::string& name, const std::string& host, int port) {

    using namespace drogon;
    ws = WebSocketClient::newWebSocketClient(host, port, false);
    auto req = HttpRequest::newHttpRequest();
    req->setPath("/");

    ws->setMessageHandler(
        [](const std::string &message, const WebSocketClientPtr &ws,
           const WebSocketMessageType &type) { std::cout << "recv msg\n"; });
    ws->setConnectionClosedHandler(
        [](const WebSocketClientPtr &ws) { std::cout << "closed\n"; });

    ws->connectToServer(req, [name](ReqResult r, const HttpResponsePtr &resp,
                                  const WebSocketClientPtr &ws) {
        auto c = ws->getConnection();
        if (r == ReqResult::Ok) {
            std::cout << "connected\n";
            c->send(Message::pack(Message::Name{name}));
        } else {
            std::cout << "error\n";
            // todo: エラー時どうするか
        }
    });

    static struct MainLoop {
        std::optional<std::thread> thr;
        MainLoop() {
            std::cout << "mainloop start" << std::endl;
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
            std::cout << "thread started" << std::endl;
        }
        ~MainLoop() {
            app().getLoop()->queueInLoop([]() { app().quit(); });
            std::cout << "mainloop quit" << std::endl;
            thr->join();
            std::cout << "thread finished" << std::endl;
        }
    } q;
}
} // namespace WebCFace
#pragma once

#include <drogon/WebSocketController.h>

using namespace drogon;

namespace WebCFace {
namespace Server {
class MainWebsock : public drogon::WebSocketController<MainWebsock> {
  public:
    void handleNewMessage(const WebSocketConnectionPtr &, std::string &&,
                          const WebSocketMessageType &) override;
    void handleNewConnection(const HttpRequestPtr &,
                             const WebSocketConnectionPtr &) override;
    void handleConnectionClosed(const WebSocketConnectionPtr &) override;
    WS_PATH_LIST_BEGIN
    // list path definitions here;
    WS_PATH_ADD("/");
    WS_PATH_LIST_END
};
} // namespace Server
} // namespace WebCFace

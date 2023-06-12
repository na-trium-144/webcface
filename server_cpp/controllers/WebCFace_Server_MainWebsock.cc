#include "WebCFace_Server_MainWebsock.h"
#include "../src/store.h"

using namespace WebCFace::Server;

void MainWebsock::handleNewMessage(const WebSocketConnectionPtr &wsConnPtr,
                                   std::string &&message,
                                   const WebSocketMessageType &type) {
    // write your application logic here
}

void MainWebsock::handleNewConnection(const HttpRequestPtr &req,
                                      const WebSocketConnectionPtr &wsConnPtr) {
    // write your application logic here
    store.newClient(wsConnPtr);
}

void MainWebsock::handleConnectionClosed(
    const WebSocketConnectionPtr &wsConnPtr) {
    // write your application logic here
}

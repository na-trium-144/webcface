#include "WebCFace_MainWebsock.h"
#include "../lib/server.hpp"
#include "../lib/client.hpp"
#include <exception>
#include <thread>

using namespace WebCFace;

void MainWebsock::handleNewMessage(
    const WebSocketConnectionPtr& wsConnPtr, std::string&& message, const WebSocketMessageType&)
{
    // 2byteの謎のバイナリデータが来ることがある
    if (message.size() <= 3)
        return;

    auto cli = clients[wsConnPtr];
    if (cli) {
        try {

            std::stringstream msg_ss(message);
            Json::Value v;
            msg_ss >> v;

            const auto msgname = v["msgname"].asString();
            const auto msg = v["msg"];

            if (msgname == "function") {
                // std::cout << message << std::endl;
                std::stringstream args_json;
                args_json << msg["args"];
                const auto func_name = msg["name"].asString();
                int func_id = msg["callback_id"].asInt();
                const auto args_str = args_json.str();

                std::thread t([=]() {
                    Client::ErrorBuffer ebuf(cli, func_id);
                    std::ostream err(&ebuf);
                    if (func_name == "") {
                        err << "funcion name is empty!" << std::endl;
                        return;
                    }
                    callToRobot(func_name, args_str, &err);
                });
                t.detach();
            } else if (msgname == "to_robot") {
                std::cerr << "to_robot is not implemented" << std::endl;
                /*                // std::cout << message << std::endl;
                                std::stringstream args_json;
                                args_json << msg["value"];
                                const auto var_name = msg["name"].asString();
                                if (var_name == "") {
                                    cli->err << "variable name is empty!" << std::endl;
                                    return;
                                }
                                changeVarToRobot(var_name, args_json.str(), cli);*/
            } else if (msgname == "gamepad") {
                cli->updateGamepad(msg);
            } else {
                /*cli->err*/ std::cerr << "Undefined message name : " << msgname << " : " << msg << std::endl;
            }
        } catch (const std::exception& e) {
            /*cli->err*/ std::cerr << e.what() << std::endl;
        }
    }
}

void MainWebsock::handleNewConnection(
    const HttpRequestPtr&, const WebSocketConnectionPtr& wsConnPtr)
{
    auto cli = std::make_shared<Client>(wsConnPtr);
    auto cli_p = std::make_pair(WebSocketConnectionPtr(wsConnPtr), cli);
    clients.insert(cli_p);
    cli->send_settings(settingJson());
    cli->send_fromRobot(fromRobotJson(false));
    cli->send_log(logJson(false));
    cli->send_layout(layoutJson(false));
}

void MainWebsock::handleConnectionClosed(const WebSocketConnectionPtr& wsConnPtr)
{
    clients.erase(wsConnPtr);
}

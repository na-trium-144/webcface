#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include <webcface/server.hpp>
#include <drogon/WebSocketController.h>
#include "client.hpp"
#include "host.hpp"

namespace WebCFace
{
inline namespace Server
{
void callToRobot(const std::string& name, const std::string& args_json, std::ostream* err);
/*void changeVarToRobot(
    const std::string& name, const std::string& args_json, const std::shared_ptr<Client>& cli);*/
std::string settingJson();
std::optional<std::string> fromRobotJson(bool changed_only);
std::optional<std::string> logJson(bool changed_only);
std::optional<std::string> layoutJson(bool changed_only);
std::optional<std::string> layerJson(bool changed_only);

std::vector<std::string> getAllRegisteredFunctions();
bool isFuncRegistered(const std::string& func_name);

inline std::unordered_map<drogon::WebSocketConnectionPtr, std::shared_ptr<Client>> clients;

inline int server_port = 0;
inline std::string server_name = "";
inline std::string getServerName()
{
    std::lock_guard lock(internal_mutex);
    if (server_name != "") {
        return server_name;
    } else {
        return host.name + "_" + std::to_string(server_port);
    }
}
struct RelatedServer {
    std::string addr;
    int port;
};
inline std::vector<RelatedServer> related_servers;
}  // namespace Server
}  // namespace WebCFace

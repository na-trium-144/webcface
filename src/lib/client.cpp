#include "client.hpp"
#include "server.hpp"
#include <iostream>
#include <json/json.h>

namespace WebCFace
{
Client::Client(drogon::WebSocketConnectionPtr ptr)
    : con(ptr), error_buffer(this), err(&error_buffer)
{
    std::cout << "[WebCface] new connection!" << std::endl;
}

Client::~Client()
{
    std::cout << "[WebCface] client removed!" << std::endl;
}
void Client::_send(const std::string& msg_name, const std::string& msg) const
{
    const auto data = "{ \"msgname\":\"" + msg_name + "\",\"msg\":" + msg + "}";
    con->send(data);
}
void Client::send_settings(const std::string& setting_str) const
{
    _send("setting", setting_str);
}
void Client::send_toRobot(const std::string& json) const
{
    _send("to_robot", json);
}
void Client::send_fromRobot(const std::string& json) const
{
    _send("from_robot", json);
}
void Client::send_image(const std::string& json) const
{
    _send("images", json);
}
void Client::send_layout(const std::string& json) const
{
    _send("layout", json);
}
void Client::send_log(const std::string& json) const
{
    _send("log", json);
}
void Client::send_error(const std::string& error) const
{
    // ダブルクオーテーションで囲うだけでなくエスケープもしないといけないので、json化
    std::stringstream ss;
    Json::Value e_json = error;
    ss << e_json;
    _send("error", ss.str());
    std::cerr << "[WebCFace] " << error;
    if (!(error.size() >= 1 && error.back() == '\n')) {
        std::cerr << '\n';
    }
    std::cerr << std::flush;
}
}  // namespace WebCFace

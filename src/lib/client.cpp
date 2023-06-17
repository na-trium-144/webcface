#include "client.hpp"
#include "server.hpp"
#include <iostream>
#include <json/json.h>

namespace WebCFace
{
Client::Client(drogon::WebSocketConnectionPtr ptr) : con(ptr)
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
void Client::send_error(int func_id, const std::string& error) const
{
    // ダブルクオーテーションで囲うだけでなくエスケープもしないといけないので、json化
    std::stringstream ss;
    Json::Value e_json = Json::objectValue;
    e_json["message"] = error;
    e_json["callback_id"] = func_id;
    ss << e_json;
    _send("error", ss.str());
    /*    std::cerr << "[WebCFace] " << error;
        if (!(error.size() >= 1 && error.back() == '\n')) {
            std::cerr << '\n';
        }
        std::cerr << std::flush;*/
}
void Client::updateGamepad(Json::Value msg)
{
    std::lock_guard lock(internal_mutex);
    gamepad_state.connected = msg["connected"].as<bool>();
    if (msg["connected"]) {
        for (const auto& button : msg["buttons"].getMemberNames()) {
            int bi = std::stoi(button);
            while (gamepad_state.buttons.size() <= bi) {
                gamepad_state.buttons.push_back(false);
            }
            gamepad_state.buttons[bi] = msg["buttons"][button].as<bool>();
        }
        for (const auto& axis : msg["axes"].getMemberNames()) {
            int ai = std::stoi(axis);
            while (gamepad_state.axes.size() <= ai) {
                gamepad_state.axes.push_back(0);
            }
            gamepad_state.axes[ai] = msg["axes"][axis].as<double>();
        }
    } else {
        gamepad_state.axes.clear();
        gamepad_state.buttons.clear();
    }
}
}  // namespace WebCFace

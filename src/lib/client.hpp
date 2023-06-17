#pragma once
#include <drogon/WebSocketController.h>
#include <ostream>
#include <streambuf>
#include <cstring>
#include <webcface/gamepad.hpp>
#include <memory>
namespace WebCFace
{
class Client
{
private:
    drogon::WebSocketConnectionPtr con;  // TODO: const
    void _send(const std::string& msg_name, const std::string& msg) const;

public:
    class ErrorBuffer : public std::streambuf
    {
        std::shared_ptr<Client> cli;
        char buf[10000] = {};
        int func_id;

    public:
        ErrorBuffer() = delete;
        ErrorBuffer(std::shared_ptr<Client> cli, int func_id) : cli(cli), func_id(func_id)
        {
            setp(buf, buf + sizeof(buf));
        }
        int sync()
        {
            cli->send_error(func_id, std::string(buf));
            std::memset(buf, 0, sizeof(buf));
            setp(buf, buf + sizeof(buf));
            return 0;
        }
    };

    void send_settings(const std::string& setting_str) const;
    void send_toRobot(const std::string& json) const;
    void send_fromRobot(const std::string& json) const;
    void send_log(const std::string& json) const;
    void send_image(const std::string& json) const;
    void send_layout(const std::string& json) const;
    void send_layer(const std::string& json) const;
    void send_dialog(const std::string& json) const;
    void send_error(int func_id, const std::string& error) const;
    void updateGamepad(Json::Value msg);
    GamepadState gamepad_state;

    Client() = delete;
    Client(const Client& other) = delete;
    Client& operator=(const Client& other) = delete;
    explicit Client(drogon::WebSocketConnectionPtr ptr);

    ~Client();
};
}  // namespace WebCFace

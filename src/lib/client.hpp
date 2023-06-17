#pragma once
#include <drogon/WebSocketController.h>
#include <ostream>
#include <streambuf>
#include <cstring>
namespace WebCFace
{
class Client
{
private:
    drogon::WebSocketConnectionPtr con;  // TODO: const
    void _send(const std::string& msg_name, const std::string& msg) const;
    class ErrorBuffer : public std::streambuf
    {
        Client* cli;
        char buf[10000] = {};

    public:
        ErrorBuffer() = delete;
        explicit ErrorBuffer(Client* cli) : cli(cli) { setp(buf, buf + sizeof(buf)); }
        int sync()
        {
            cli->send_error(std::string(buf));
            std::memset(buf, 0, sizeof(buf));
            setp(buf, buf + sizeof(buf));
            return 0;
        }
    } error_buffer;

public:
    void send_settings(const std::string& setting_str) const;
    void send_toRobot(const std::string& json) const;
    void send_fromRobot(const std::string& json) const;
    void send_log(const std::string& json) const;
    void send_image(const std::string& json) const;
    void send_layout(const std::string& json) const;
    void send_error(const std::string& error) const;
    // send_errorを使用する代わりに err << エラーメッセージ << std::endl; でよい
    std::ostream err;
    Client() = delete;
    Client(const Client& other) = delete;
    Client& operator=(const Client& other) = delete;
    explicit Client(drogon::WebSocketConnectionPtr ptr);

    ~Client();
};
}  // namespace WebCFace

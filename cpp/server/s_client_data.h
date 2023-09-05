#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
#include "../message/message.h"

namespace WebCFace::Server {
class ClientData {
  public:
    using wsConnPtr = std::shared_ptr<void>;

  private:
    const wsConnPtr con;
    bool connected() const;

    std::string name;
    //! 最新の値
    std::unordered_map<std::string, double> value;
    std::unordered_map<std::string, std::string> text;
    std::unordered_map<std::string, Message::FuncInfo> func;
    std::unordered_map<std::string, std::vector<ViewComponentBase>> view;
    std::chrono::system_clock::time_point last_sync_time;
    //! リクエストしているmember,nameのペア
    std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>> value_req;
    std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>> text_req;
    std::unordered_map<std::string, std::unordered_map<std::string, unsigned int>> view_req;
    std::set<std::string> log_req;
    bool hasReq(const std::string &member);
    //! ログ全履歴
    std::vector<Message::Log::LogLine> log;

  public:
    //! 初回のsync()が終わったか
    //! falseならentryの通知などはしない
    bool sync_init = false;
    
    ClientData() = delete;
    ClientData(const ClientData &) = delete;
    ClientData &operator=(const ClientData &) = delete;
    explicit ClientData(const wsConnPtr &con) : con(con) {}

    void onConnect();
    void onRecv(const std::string &msg);
    void onClose();

    std::stringstream send_buffer;
    int send_len;
    void send();
    template <typename T>
    void pack(const T &data) {
        Message::pack(send_buffer, send_len, data);
    }
};
} // namespace WebCFace::Server

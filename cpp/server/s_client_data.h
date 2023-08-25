#pragma once
#include <drogon/WebSocketController.h>
#include <string>
#include <unordered_map>
#include <set>
#include "../message/message.h"

namespace WebCFace::Server {
class ClientData {
  public:
    using wsConnPtr = drogon::WebSocketConnectionPtr;

  private:
    const wsConnPtr con;
    bool connected() const;

    //! 初回のsync()が終わったか
    //! falseならentryの通知などはしない
    bool sync_init = false;
    std::string name;
    //! 最新の値
    std::unordered_map<std::string, double> value;
    std::unordered_map<std::string, std::string> text;
    std::unordered_map<std::string, Message::FuncInfo> func;
    std::unordered_map<std::string, std::vector<ViewComponent>> view;
    //! リクエストしているmember,nameのペア
    std::set<std::pair<std::string, std::string>> value_subsc;
    std::set<std::pair<std::string, std::string>> text_subsc;
    std::set<std::pair<std::string, std::string>> view_subsc;
    std::set<std::string> log_subsc;
    //! ログ全履歴
    std::vector<Message::Log::LogLine> log;

  public:
    ClientData() = delete;
    ClientData(const ClientData &) = delete;
    ClientData &operator=(const ClientData &) = delete;
    explicit ClientData(const wsConnPtr &con) : con(con) {}

    void onConnect();
    void onRecv(const std::string &msg);
    void onClose();
    void send(const std::vector<char> &m) const;
};
} // namespace WebCFace::Server

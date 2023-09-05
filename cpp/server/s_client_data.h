#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
#include "../message/message.h"
#include <spdlog/common.h>
#include <spdlog/logger.h>

namespace WebCFace::Server {
struct ClientData {
    using wsConnPtr = std::shared_ptr<void>;
    spdlog::sink_ptr sink;
    std::shared_ptr<spdlog::logger> logger;
    spdlog::level::level_enum logger_level;

    const wsConnPtr con;
    bool connected() const;

    //! 初回のsync()が終わったか
    //! falseならentryの通知などはしない
    bool sync_init = false;

    std::string name;
    unsigned int member_id;
    //! 最新の値
    std::unordered_map<std::string, double> value;
    std::unordered_map<std::string, std::string> text;
    std::unordered_map<std::string, Message::FuncInfo> func;
    std::unordered_map<std::string, std::vector<ViewComponentBase>> view;
    std::chrono::system_clock::time_point last_sync_time;
    //! リクエストしているmember,nameのペア
    std::unordered_map<std::string,
                       std::unordered_map<std::string, unsigned int>>
        value_req, text_req, view_req;
    std::set<std::string> log_req;
    bool hasReq(const std::string &member);
    //! ログ全履歴
    std::vector<Message::Log::LogLine> log;

    ClientData() = delete;
    ClientData(const ClientData &) = delete;
    ClientData &operator=(const ClientData &) = delete;
    explicit ClientData(const wsConnPtr &con, const spdlog::sink_ptr &sink,
                        spdlog::level::level_enum level)
        : con(con), sink(sink), logger_level(level) {
        static unsigned int new_member_id = 0;
        this->member_id = ++new_member_id;
        logger = std::make_shared<spdlog::logger>(
            std::to_string(member_id) + "_(unknown client)", this->sink);
        logger->set_level(this->logger_level);
    }

    void onConnect();
    void onRecv(const std::string &msg);
    void onClose();

    std::stringstream send_buffer;
    int send_len = 0;
    void send();
    template <typename T>
    void pack(const T &data) {
        Message::pack(send_buffer, send_len, data);
    }
};
} // namespace WebCFace::Server

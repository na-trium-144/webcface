#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
#include <optional>
#include "../message/message.h"
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <webcface/common/def.h>

namespace WEBCFACE_NS::Server {
struct ClientData {
    using wsConnPtr = void *;
    spdlog::sink_ptr sink;
    std::shared_ptr<spdlog::logger> logger;
    spdlog::level::level_enum logger_level;

    /*!
     * \brief ws接続のポインタ、切断後(onClose後)nullptrになる
     */
    wsConnPtr con;
    std::string remote_addr;
    bool connected() const { return con != nullptr; }

    /*!
     * \brief 初回のsync() が終わったか
     *
     * falseならentryの通知などはしない
     *
     */
    bool sync_init = false;

    std::string name;
    unsigned int member_id;
    Message::SyncInit init_data;
    //! 最新の値
    std::unordered_map<std::string, std::shared_ptr<std::vector<double>>> value;
    std::unordered_map<std::string, std::shared_ptr<std::string>> text;
    std::unordered_map<std::string, std::shared_ptr<Message::FuncInfo>> func;
    std::unordered_map<std::string, std::vector<Common::ViewComponentBase>>
        view;
    std::chrono::system_clock::time_point last_sync_time;
    //! リクエストしているmember,nameのペア
    std::unordered_map<std::string,
                       std::unordered_map<std::string, unsigned int>>
        value_req, text_req, view_req;
    std::set<std::string> log_req;
    bool hasReq(const std::string &member);
    //! ログ全履歴
    std::shared_ptr<std::deque<Message::Log::LogLine>> log;
    /*!
     * \brief まだ完了していない自分へのcall呼び出しのリスト
     *
     * [caller_member_id][caller_id]
     *
     * 呼び出し開始で2, response返したら1, result返したら0
     *
     * 切断時にそれぞれにresponseを返す必要がある。
     *
     */
    std::unordered_map<unsigned int, std::unordered_map<std::size_t, int>>
        pending_calls;

    inline static unsigned int last_member_id = 0;

    ClientData() = delete;
    ClientData(const ClientData &) = delete;
    ClientData &operator=(const ClientData &) = delete;
    explicit ClientData(const wsConnPtr &con, const std::string &remote_addr,
                        const spdlog::sink_ptr &sink,
                        spdlog::level::level_enum level)
        : sink(sink), logger_level(level), con(con), remote_addr(remote_addr),
          log(std::make_shared<std::deque<Message::Log::LogLine>>()) {
        this->member_id = ++last_member_id;
        logger = std::make_shared<spdlog::logger>(
            std::to_string(member_id) + "_(unknown client)", this->sink);
        logger->set_level(this->logger_level);
    }

    void onConnect();
    void onRecv(const std::string &msg);
    void onClose();

    void sendPing();
    static constexpr std::chrono::milliseconds ping_interval{5000};
    std::chrono::system_clock::time_point last_send_ping;
    std::optional<std::chrono::milliseconds> last_ping_duration;
    bool ping_status_req = false;

    std::stringstream send_buffer;
    int send_len = 0;
    void send();
    void send(const std::string &msg);

    template <typename T>
    void pack(const T &data) {
        Message::pack(send_buffer, send_len, data);
    }
};
} // namespace WEBCFACE_NS::Server

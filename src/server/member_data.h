#pragma once
#include <string>
#include <set>
#include <unordered_map>
#include <chrono>
#include <optional>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include "../message/message.h"
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <webcface/common/def.h>
#include <webcface/server.h>

WEBCFACE_NS_BEGIN
namespace Server {
struct WEBCFACE_DLL MemberData {
    spdlog::sink_ptr sink;
    std::shared_ptr<spdlog::logger> logger;
    spdlog::level::level_enum logger_level;

    ServerStorage *store;

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

    std::u8string name;
    std::string name_s; // decoded
    unsigned int member_id;
    Message::SyncInit init_data;
    /*!
     * \brief 最新の値
     *
     * entry非表示のものも含む。
     *
     */
    std::unordered_map<std::u8string, std::shared_ptr<std::vector<double>>>
        value;
    std::unordered_map<std::u8string, std::shared_ptr<Common::ValAdaptor>> text;
    std::unordered_map<std::u8string, std::shared_ptr<Message::FuncInfo>> func;
    std::unordered_map<std::u8string, std::vector<Common::ViewComponentBase>>
        view;
    std::unordered_map<std::u8string,
                       std::vector<Common::Canvas3DComponentBase>>
        canvas3d;
    std::unordered_map<std::u8string, Common::Canvas2DDataBase> canvas2d;
    std::unordered_map<std::u8string, Common::ImageBase> image;
    std::unordered_map<std::u8string, int> image_changed;
    // 画像が変化したことを知らせるcv
    std::unordered_map<std::u8string, std::mutex> image_m;
    std::unordered_map<std::u8string, std::condition_variable> image_cv;

    std::chrono::system_clock::time_point last_sync_time;
    //! リクエストしているmember,nameのペア
    std::unordered_map<std::u8string,
                       std::unordered_map<std::u8string, unsigned int>>
        value_req, text_req, view_req, image_req, robot_model_req, canvas3d_req,
        canvas2d_req;
    // リクエストが変化したことをスレッドに知らせる
    std::unordered_map<std::u8string, std::unordered_map<std::u8string, int>>
        image_req_changed;
    // 画像をそれぞれのリクエストに合わせて変換するスレッド
    std::unordered_map<std::u8string,
                       std::unordered_map<std::u8string, Common::ImageReq>>
        image_req_info;
    std::unordered_map<
        std::u8string,
        std::unordered_map<std::u8string, std::optional<std::thread>>>
        image_convert_thread;
    void imageConvertThreadMain(const std::u8string &member,
                                const std::u8string &field);
    std::unordered_map<
        std::u8string,
        std::shared_ptr<std::vector<Message::RobotModel::RobotLink>>>
        robot_model;

    std::set<std::u8string> log_req;
    bool hasReq(const std::u8string &member);
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

    MemberData() = delete;
    MemberData(const MemberData &) = delete;
    MemberData &operator=(const MemberData &) = delete;
    explicit MemberData(ServerStorage *store, const wsConnPtr &con,
                        std::string_view remote_addr,
                        const spdlog::sink_ptr &sink,
                        spdlog::level::level_enum level)
        : sink(sink), logger_level(level), store(store), con(con),
          remote_addr(remote_addr),
          log(std::make_shared<std::deque<Message::Log::LogLine>>()) {
        this->member_id = ++last_member_id;
        logger = std::make_shared<spdlog::logger>(
            std::to_string(member_id) + "_(unknown client)", this->sink);
        logger->set_level(this->logger_level);
    }
    ~MemberData() { onClose(); }

    void onConnect();
    void onRecv(const std::string &msg);
    void onClose();
    std::atomic<bool> closing = false;

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
} // namespace Server
WEBCFACE_NS_END

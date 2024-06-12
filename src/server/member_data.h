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

    SharedString name;
    std::string name_s; // decoded
    unsigned int member_id;
    Message::SyncInit init_data;
    /*!
     * \brief 最新の値
     *
     * entry非表示のものも含む。
     *
     */
    std::unordered_map<SharedString, std::shared_ptr<std::vector<double>>,
                       SharedString::Hash>
        value;
    std::unordered_map<SharedString, std::shared_ptr<Common::ValAdaptor>,
                       SharedString::Hash>
        text;
    std::unordered_map<SharedString, std::shared_ptr<Message::FuncInfo>,
                       SharedString::Hash>
        func;
    std::unordered_map<SharedString, std::vector<Common::ViewComponentBase>,
                       SharedString::Hash>
        view;
    std::unordered_map<SharedString, std::vector<Common::Canvas3DComponentBase>,
                       SharedString::Hash>
        canvas3d;
    std::unordered_map<SharedString, Common::Canvas2DDataBase,
                       SharedString::Hash>
        canvas2d;
    std::unordered_map<SharedString, Common::ImageBase, SharedString::Hash>
        image;
    std::unordered_map<SharedString, int, SharedString::Hash> image_changed;
    // 画像が変化したことを知らせるcv
    std::unordered_map<SharedString, std::mutex, SharedString::Hash> image_m;
    std::unordered_map<SharedString, std::condition_variable,
                       SharedString::Hash>
        image_cv;

    std::chrono::system_clock::time_point last_sync_time;
    //! リクエストしているmember,nameのペア
    std::unordered_map<
        SharedString,
        std::unordered_map<SharedString, unsigned int, SharedString::Hash>,
        SharedString::Hash>
        value_req, text_req, view_req, image_req, robot_model_req, canvas3d_req,
        canvas2d_req;
    // リクエストが変化したことをスレッドに知らせる
    std::unordered_map<
        SharedString, std::unordered_map<SharedString, int, SharedString::Hash>,
        SharedString::Hash>
        image_req_changed;
    // 画像をそれぞれのリクエストに合わせて変換するスレッド
    std::unordered_map<
        SharedString,
        std::unordered_map<SharedString, Common::ImageReq, SharedString::Hash>,
        SharedString::Hash>
        image_req_info;

    // image_convert_thread[imageのmember][imageのfield] =
    // imageを変換してthisに送るスレッド
    std::unordered_map<
        SharedString,
        std::unordered_map<SharedString, std::optional<std::thread>,
                           SharedString::Hash>,
        SharedString::Hash>
        image_convert_thread;
    void imageConvertThreadMain(const SharedString &member,
                                const SharedString &field);
    std::unordered_map<
        SharedString,
        std::shared_ptr<std::vector<Message::RobotModel::RobotLink>>,
        SharedString::Hash>
        robot_model;

    std::unordered_set<SharedString, SharedString::Hash> log_req;
    bool hasReq(const SharedString &member);
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

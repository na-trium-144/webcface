#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <chrono>
#include <optional>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <spdlog/common.h>
#include <spdlog/logger.h>
#include "webcface/common/encoding.h"
#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/common/internal/message/func.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/common/internal/message/log.h"
#include "webcface/common/internal/message/robot_model.h"
#include "webcface/common/internal/message/sync.h"
#include "webcface/common/internal/message/view.h"
#include "webcface/server/server.h"

WEBCFACE_NS_BEGIN
namespace server {

std::pair<unsigned int, SharedString> findReqField(StrMap2<unsigned int> &req,
                                                   const SharedString &member,
                                                   const SharedString &field);

struct MemberData {
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
    unsigned int member_id;
    message::SyncInit init_data;
    /*!
     * \brief 最新の値
     *
     * entry非表示のものも含む。
     *
     */
    StrMap1<std::shared_ptr<std::vector<double>>> value;
    StrMap1<std::shared_ptr<ValAdaptor>> text;
    StrMap1<std::shared_ptr<message::FuncInfo>> func;
    StrMap1<message::ViewData> view;
    StrMap1<message::Canvas3DData> canvas3d;
    StrMap1<message::Canvas2DData> canvas2d;

    StrMap1<message::ImageFrame> image;
    /*!
     * 画像が変化したことを知らせるcv
     * リクエストする側のcvに対して、リクエストする側も画像送信側もnotifyする
     * * image_req_changedを変えるときnotify
     * * image_changedが変えるとき全メンバーの中でこれをリクエストしてるやつをすべてnotify
     * * closingのときnotify、全メンバーの中でこれをリクエストしてるやつをすべてnotify
     *
     * \todo 今後いつかマルチスレッド化したときにデッドロックする気がする
     */
    std::condition_variable image_cv;
    std::mutex image_m;
    /*!
     * 自分の画像が変化したことをスレッドに知らせる
     * リクエストされてるメンバーのcvを起こしに行く
     */
    StrMap1<int> image_changed;
    /*!
     * リクエストが変化したことをスレッドに知らせる
     */
    StrMap2<int> image_req_changed;
    /*!
     * 画像をそれぞれのリクエストに合わせて変換するスレッド
     * (リクエスト側がもつ)
     */
    StrMap2<message::ImageReq> image_req_info;

    std::chrono::system_clock::time_point last_sync_time;
    //! リクエストしているmember,nameのペア
    StrMap2<unsigned int> value_req, text_req, view_req, image_req,
        view_old_req, robot_model_req, canvas3d_req, canvas3d_old_req,
        canvas2d_req, canvas2d_old_req, log_req;

    // image_convert_thread[imageのmember][imageのfield] =
    // imageを変換してthisに送るスレッド
    StrMap2<std::optional<std::thread>> image_convert_thread;
    void imageConvertThreadMain(const SharedString &member,
                                const SharedString &field);
    StrMap1<std::vector<std::shared_ptr<message::RobotLink>>> robot_model;

    bool hasReq(const SharedString &member);
    //! 古いLogリクエスト ("default"のログを古いメッセージ形式で返す)
    StrSet1 log_req_default;
    //! ログ全履歴
    StrMap1<std::shared_ptr<std::deque<message::LogLine>>> log;
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
          remote_addr(remote_addr) {
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
        message::pack(send_buffer, send_len, data);
    }
};
} // namespace server
WEBCFACE_NS_END

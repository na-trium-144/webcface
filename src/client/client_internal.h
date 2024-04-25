#pragma once
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <cstdlib>
#include <eventpp/eventdispatcher.h>
#include <spdlog/logger.h>
#include <webcface/common/vector.h>
#include <webcface/field.h>
#include <webcface/common/func.h>
#include <webcface/common/view.h>
#include <webcface/common/log.h>
#include <webcface/common/queue.h>
#include <webcface/common/image.h>
#include <webcface/common/robot_model.h>
#include <webcface/common/canvas3d.h>
#include <webcface/common/canvas2d.h>
#include <webcface/func_result.h>
#include <webcface/logger.h>
#include "data_store1.h"
#include "data_store2.h"
#include "func_internal.h"
#include <webcface/common/def.h>
#include <webcface/encoding.h>


WEBCFACE_NS_BEGIN
namespace Internal {

WEBCFACE_DLL void messageThreadMain(std::shared_ptr<ClientData> data,
                                    std::u8string host, int port);

WEBCFACE_DLL void recvThreadMain(std::shared_ptr<ClientData> data);

struct ClientData : std::enable_shared_from_this<ClientData> {
    explicit ClientData(std::string &name, const std::string &host = "",
                        int port = -1)
        : ClientData(Encoding::initName(name), Encoding::initName(host), port) {}
    explicit ClientData(std::wstring &name, const std::wstring &host = L"",
                        int port = -1)
        : ClientData(Encoding::initNameW(name), Encoding::initNameW(host), port) {}

    WEBCFACE_DLL explicit ClientData(const std::u8string &name,
                                     const std::u8string &host = u8"",
                                     int port = -1);

    /*!
     * \brief 1回以上参照されたメンバー名とフィールド名の文字列本体を保持する
     *
     * null終端にすること
     *
     */
    std::vector<std::unique_ptr<char8_t[]>> members, fields;
    WEBCFACE_DLL MemberNameRef getMemberRef(std::u8string_view name);
    MemberNameRef getMemberRef(std::string_view name) {
        return getMemberRef(Encoding::initName(name));
    }
    MemberNameRef getMemberRef(std::wstring_view name) {
        return getMemberRef(Encoding::initNameW(name));
    }
    WEBCFACE_DLL FieldNameRef getFieldRef(std::u8string_view name);
    FieldNameRef getFieldRef(std::string_view name) {
        return getFieldRef(Encoding::initName(name));
    }
    FieldNameRef getFieldRef(std::wstring_view name) {
        return getFieldRef(Encoding::initNameW(name));
    }
    std::mutex name_mtx;

    /*!
     * \brief Client自身の名前
     *
     */
    MemberNameRef self_member_name;
    bool isSelf(const Field &base) const {
        return base.memberPtr() == self_member_name;
    }

    std::u8string host;
    int port;

    /*!
     * \brief websocket通信するスレッド
     *
     */
    std::unique_ptr<std::thread> message_thread;
    /*!
     * \brief recv_queueを処理するスレッド
     *
     */
    std::unique_ptr<std::thread> recv_thread;

    /*!
     * \brief close()が呼ばれたらtrue
     *
     */
    std::atomic<bool> closing = false;
    std::atomic<bool> connected = false;
    std::mutex connect_state_m;
    std::condition_variable connect_state_cond;

    /*!
     * \brief 通信関係のスレッドを開始する
     *
     */
    WEBCFACE_DLL void start();
    /*!
     * \brief threadを待機 (close時)
     *
     */
    WEBCFACE_DLL void join();

    /*!
     * \brief 初期化時に送信するメッセージをキューに入れる
     *
     * 各種req と syncData(true) の全データが含まれる。
     *
     * コンストラクタ直後start()前と、切断直後に生成してキューの最初に入れる
     *
     */
    WEBCFACE_DLL void syncDataFirst();
    /*!
     * \brief sync() 1回分のメッセージをキューに入れる
     *
     * value, text, view, log, funcの送信データの前回からの差分が含まれる。
     * 各種reqはsyncとは無関係に送信される
     *
     * \param is_first trueのとき差分ではなく全データを送る
     * (syncDataFirst()内から呼ばれる)
     *
     */
    WEBCFACE_DLL void syncData(bool is_first);

    /*!
     * \brief 送信したいメッセージを入れるキュー
     *
     * 接続できていない場合送信されずキューにたまる
     *
     */
    std::shared_ptr<Common::Queue<std::string>> message_queue;
    /*!
     * \brief wsが受信したメッセージを入れるキュ
     *
     */
    Common::Queue<std::string> recv_queue;

    /*!
     * \brief 受信時の処理
     *
     */
    WEBCFACE_DLL void onRecv(const std::string &message);

    std::mutex entry_m;
    std::unordered_set<std::string> member_entry;
    SyncDataStore2<ValueData> value_store;
    SyncDataStore2<TextData> text_store;
    SyncDataStore2<FuncData> func_store;
    SyncDataStore2<ViewData> view_store;
    SyncDataStore2<ImageData, Common::ImageReq> image_store;
    SyncDataStore2<RobotModelData> robot_model_store;
    SyncDataStore2<Canvas3DData> canvas3d_store;
    SyncDataStore2<Canvas2DData> canvas2d_store;
    std::shared_ptr<SyncDataStore1<std::shared_ptr<std::vector<LogLine>>>>
        log_store;
    SyncDataStore1<std::chrono::system_clock::time_point> sync_time_store;
    FuncResultStore func_result_store;
    std::size_t log_sent_lines = 0;

    /*!
     * \brief listenerがfetchするの待ちの関数呼び出しをためておく
     *
     */
    std::unordered_map<MemberNameRef, Common::Queue<FuncCallHandle>>
        func_listener_handlers;

    std::unordered_map<MemberNameRef, unsigned int> member_ids;
    std::unordered_map<unsigned int, std::string> member_lib_name,
        member_lib_ver, member_addr;
    MemberNameRef getMemberNameFromId(unsigned int id) {
        std::lock_guard lock(entry_m);
        for (const auto &it : member_ids) {
            if (it.second == id) {
                return it.first;
            }
        }
        return "";
    }
    unsigned int getMemberIdFromName(MemberNameRef name) {
        std::lock_guard lock(entry_m);
        auto it = member_ids.find(name);
        if (it != member_ids.end()) {
            return it->second;
        }
        return 0;
    }

    std::mutex event_m;
    std::map<FieldComparable, eventpp::CallbackList<void(Value)>>
        value_change_event;
    std::map<FieldComparable, eventpp::CallbackList<void(Text)>>
        text_change_event;
    std::map<FieldComparable, eventpp::CallbackList<void(Image)>>
        image_change_event;
    std::map<FieldComparable, eventpp::CallbackList<void(RobotModel)>>
        robot_model_change_event;
    std::map<FieldComparable, eventpp::CallbackList<void(View)>>
        view_change_event;
    std::map<FieldComparable, eventpp::CallbackList<void(Canvas3D)>>
        canvas3d_change_event;
    std::map<FieldComparable, eventpp::CallbackList<void(Canvas2D)>>
        canvas2d_change_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Log)>>
        log_append_event;
    eventpp::CallbackList<void(Member)> member_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Member)>>
        sync_event, ping_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Value)>>
        value_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Text)>>
        text_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Func)>>
        func_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(View)>>
        view_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Image)>>
        image_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(RobotModel)>>
        robot_model_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Canvas3D)>>
        canvas3d_entry_event;
    std::unordered_map<MemberNameRef, eventpp::CallbackList<void(Canvas2D)>>
        canvas2d_entry_event;

    /*!
     * \brief sync()のタイミングで実行を同期する関数のcondition_variable
     *
     */
    Common::Queue<std::shared_ptr<FuncOnSync>> func_sync_queue;

    FuncWrapperType default_func_wrapper;

    std::shared_ptr<LoggerSink> logger_sink;
    std::shared_ptr<spdlog::logger> logger, logger_internal;
    std::unique_ptr<LoggerBuf> logger_buf;
    std::unique_ptr<std::ostream> logger_os;

    std::string svr_name, svr_version;

    std::shared_ptr<std::unordered_map<unsigned int, int>> ping_status =
        nullptr;
    bool ping_status_req = false;
    /*!
     * \brief ping_status_reqをtrueにしmessage_queueに投げる
     *
     */
    WEBCFACE_DLL void pingStatusReq();
};
} // namespace Internal
WEBCFACE_NS_END

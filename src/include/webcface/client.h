#pragma once
#include <string>
#include <memory>
#include <future>
#include <vector>
#include <thread>
#include "client_data.h"
#include "member.h"
#include "event_target.h"
#include "func.h"
#include "logger.h"
#include "common/def.h"

namespace WebCFace {
//! サーバーに接続するクライアント。
class Client : public Member {
  private:
    //! サーバーのホスト
    std::string host;
    //! サーバーのポート
    int port;
    //! websocket通信するスレッド
    std::thread message_thread;
    //! recv_queueを処理するスレッド
    std::thread recv_thread;
    static void messageThreadMain(std::shared_ptr<ClientData> data,
                                  std::string host, int port);
    //! 接続を切り、今後再接続しない
    void close();

    std::shared_ptr<ClientData> data;

    LoggerBuf logger_buf;
    std::ostream logger_os;

    //! 受信時の処理
    void onRecv(const std::string &message);

  public:
    Client() : Client("") {}
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    //! 自分自身のmemberとしての名前を指定しサーバーに接続する
    //! サーバーのホストとポートを省略した場合localhost:7530になる
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1",
                    int port = WEBCFACE_DEFAULT_PORT)
        : Client(name, host, port, std::make_shared<ClientData>(name)) {}
    explicit Client(const std::string &name, const std::string &host, int port,
                    std::shared_ptr<ClientData> data);
    //! サーバーに接続できているときtrueを返す。
    bool connected() const;
    //! デストラクタで接続を切る。
    ~Client();

    //! データをまとめて送信する。
    /*! value,textにセットしたデータをすべて送る。
     * 他クライアントのvalue,textを参照する場合、そのリクエストを送るのもsync()で行う。
     * また他memberの情報を取得できるのは初回のsync()の後のみ。
     * clientを使用する時は必ずsendを適当なタイミングで繰り返し呼ぶこと。
     */
    void sync();

    //! 他のmemberにアクセスする。
    Member member(const std::string &name) { return Member{data, name}; }
    //! サーバーに接続されている他のmemberのリストを得る。
    std::vector<Member> members() {
        auto keys = data->value_store.getMembers();
        std::vector<Member> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = member(keys[i]);
        }
        return ret;
    }
    //! Memberが追加された時のイベントを設定
    /*! このクライアントが接続する前から存在したメンバーについては
     * 初回の sync() 後に一度に送られるので、
     * eventの設定は初回のsync()より前に行うと良い
     */
    auto onMemberEntry() {
        return EventTarget<Member, int>{&data->member_entry_event, 0};
    }

    /*!
     * これ以降セットするFuncのデフォルトのFuncWrapperをセットする。(初期状態はnullptr)
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     */
    void setDefaultRunCond(FuncWrapperType wrapper) {
        data->default_func_wrapper = wrapper;
    }
    /*! デフォルトのFuncWrapperを nullptr にする
     */
    void setDefaultRunCondNone() { setDefaultRunCond(nullptr); }
    /*! デフォルトのFuncWrapperを runCondOnSync() にする
     */
    void setDefaultRunCondOnSync() {
        setDefaultRunCond(FuncWrapper::runCondOnSync(data));
    }
    /*! デフォルトのFuncWrapperを runCondScopeGuard() にする
     */
    template <typename ScopeGuard>
    void setDefaultRunCondScopeGuard() {
        setDefaultRunCond(FuncWrapper::runCondScopeGuard<ScopeGuard>());
    }

    //! サーバーに送信するspdlogのsink
    std::shared_ptr<LoggerSink> logger_sink() { return data->logger_sink; }
    //! サーバーとstderr_sinkに流すspdlog::logger
    std::shared_ptr<spdlog::logger> logger() { return data->logger; }

    //! このクライアントのloggerに出力するstreambuf
    //! levelは常にinfoになる (変えられるようにする?)
    //! std::flushのタイミングとは無関係に、1つの改行ごとに1つのログになる
    LoggerBuf *logger_streambuf() { return &logger_buf; }
    //! logger_streambufに出力するostream
    std::ostream &logger_ostream() { return logger_os; }

    //! WebCFaceサーバーのバージョン情報
    std::string serverVersion() const { return data->svr_version; }
    //! WebCFaceサーバーの識別情報
    std::string serverName() const { return data->svr_name; }
};

} // namespace WebCFace
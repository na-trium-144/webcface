#pragma once
#include <string>
#include <memory>
#include <future>
#include <vector>
#include <thread>
#include <atomic>
#include "client_data.h"
#include "member.h"
#include "event_target.h"
#include "func.h"
#include "logger.h"

namespace drogon {
class WebSocketClient;
}

namespace WebCFace {
//! サーバーに接続するクライアント。
class Client : public Member {
  private:
    std::shared_ptr<ClientData> data;

    std::shared_ptr<drogon::WebSocketClient> ws;
    //! close()が呼ばれたらtrue
    std::atomic<bool> closing = false;
    //! 接続が完了したかどうかを取得する
    std::future<void> connection_finished;
    //! 再接続
    //! 切れたら再帰的に呼ばれる(正確には別スレッドで呼び出されるので再帰ではない)
    void reconnect();

    //! サーバーのホスト
    std::string host;
    //! サーバーのポート
    int port;
    //! 初回のsync()で名前を送信するがそれが完了したかどうか
    bool sync_init = false;

    //! イベントを処理するスレッド
    std::thread event_thread;
    //! message_queueを処理するスレッド
    std::thread message_thread;

    LoggerBuf logger_buf;
    std::ostream logger_os;

    //! 受信時の処理
    void onRecv(const std::string &message);
    //! データを送信する
    void send(const std::vector<char> &m);
    //! 接続を切り、今後再接続しない
    void close();

  public:
    Client() : Client("") {}
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;
    //! 自分自身のmemberとしての名前を指定しサーバーに接続する
    //! サーバーのホストとポートを省略した場合localhost:7530になる
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1", int port = 7530);
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
    EventTarget<Member> membersChange() {
        return EventTarget<Member>{EventType::member_entry, data};
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
};

} // namespace WebCFace

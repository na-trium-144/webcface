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
    WEBCFACE_DLL static void messageThreadMain(std::shared_ptr<ClientData> data,
                                               std::string host, int port);
    std::shared_ptr<ClientData> data;

    LoggerBuf logger_buf;
    std::ostream logger_os;

    //! 受信時の処理
    WEBCFACE_DLL void onRecv(const std::string &message);

  public:
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;

    //! 名前を指定せずサーバーに接続する
    /*! サーバーのホストとポートはlocalhost:7530になる
     */
    Client() : Client("") {}
    //! 名前を指定しサーバーに接続する
    /*! サーバーのホストとポートを省略した場合localhost:7530になる
     *
     * \arg name 名前
     * \arg host サーバーのアドレス
     * \arg port サーバーのポート
     */
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1",
                    int port = WEBCFACE_DEFAULT_PORT)
        : Client(name, host, port, std::make_shared<ClientData>(name)) {}

    WEBCFACE_DLL explicit Client(const std::string &name,
                                 const std::string &host, int port,
                                 std::shared_ptr<ClientData> data);

    //! サーバーに接続できているときtrueを返す
    WEBCFACE_DLL bool connected() const;
    //! 接続を切りClientを破棄
    WEBCFACE_DLL ~Client();
    //! 接続を切り、今後再接続しない
    WEBCFACE_DLL void close();

    //! 送信用にセットしたデータとリクエストデータをすべて送信キューに入れる。
    /*!
     * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
     *
     * * 他memberの情報を取得できるのは初回のsync()の後のみ。
     * * 関数の呼び出しと結果の受信はsync()とは非同期に行われる。
     */
    WEBCFACE_DLL void sync();

    //! 他のmemberにアクセスする
    /*!
     * \sa members(), onMemberEntry()
     */
    Member member(const std::string &name) { return Member{data, name}; }
    //! サーバーに接続されている他のmemberのリストを得る。
    /*! 自分自身と、無名のmemberを除く。
     * \sa member(), onMemberEntry()
     */
    std::vector<Member> members() {
        auto keys = data->value_store.getMembers();
        std::vector<Member> ret(keys.size());
        for (std::size_t i = 0; i < keys.size(); i++) {
            ret[i] = member(keys[i]);
        }
        return ret;
    }
    //! Memberが追加された時のイベント
    /*! コールバックの型は void(Member)
     *
     * \sa member(), members()
     */
    EventTarget<Member, int> onMemberEntry() {
        return EventTarget<Member, int>{&data->member_entry_event, 0};
    }

    //! これ以降セットするFuncのデフォルトのFuncWrapperをセットする。(初期状態はnullptr)
    /*!
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     */
    void setDefaultRunCond(FuncWrapperType wrapper) {
        data->default_func_wrapper = wrapper;
    }
    //! デフォルトのFuncWrapperを nullptr にする
    void setDefaultRunCondNone() { setDefaultRunCond(nullptr); }
    //! デフォルトのFuncWrapperを runCondOnSync() にする
    void setDefaultRunCondOnSync() {
        setDefaultRunCond(FuncWrapper::runCondOnSync(data));
    }
    //! デフォルトのFuncWrapperを runCondScopeGuard() にする
    template <typename ScopeGuard>
    void setDefaultRunCondScopeGuard() {
        setDefaultRunCond(FuncWrapper::runCondScopeGuard<ScopeGuard>());
    }

    //! webcfaceに出力するsink
    /*! (v1.0.1で logger_sink から名前変更)
     * \sa logger(), loggerStreamBuf(), loggerOStream()
     */
    std::shared_ptr<LoggerSink> loggerSink() { return data->logger_sink; }
    //! webcfaceとstderr_sinkに出力するlogger
    /*! 初期状態では logger()->sinks() = { loggerSink(), stderr_color_sink_mt }
     * となっている
     *
     * \sa loggerSink(), loggerStreamBuf(), loggerOStream()
     */
    std::shared_ptr<spdlog::logger> logger() { return data->logger; }

    //! webcfaceに出力するstreambuf
    /*! (v1.0.1で logger_streambuf から名前変更)
     *
     * levelは常にinfoになる。
     * std::flushのタイミングとは無関係に、1つの改行ごとに1つのログになる
     *
     * \sa loggerSink(), logger(), loggerOStream()
     */
    LoggerBuf *loggerStreamBuf() { return &logger_buf; }
    //! webcfaceに出力するostream
    /*! (v1.0.1で logger_ostream から名前変更)
     * \sa loggerSink(), logger(), loggerStreamBuf()
     */
    std::ostream &loggerOStream() { return logger_os; }

    //! WebCFaceサーバーのバージョン情報
    std::string serverVersion() const { return data->svr_version; }
    //! WebCFaceサーバーの識別情報
    /*!
     * \return webcface付属のサーバーであれば通常は "webcface" が返る
     * \sa serverVersion()
     */
    std::string serverName() const { return data->svr_name; }
};

} // namespace WebCFace

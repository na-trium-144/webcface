#pragma once
#include <string>
#include <memory>
#include <future>
#include <vector>
#include <thread>
#include "member.h"
#include "event_target.h"
#include "func.h"
#include "logger.h"
#include "common/def.h"
#include <spdlog/logger.h>

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
    WEBCFACE_DLL static void
    messageThreadMain(std::shared_ptr<Internal::ClientData> data,
                      std::string host, int port);
    std::shared_ptr<Internal::ClientData> data;

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
    WEBCFACE_DLL explicit Client(const std::string &name,
                                 const std::string &host = "127.0.0.1",
                                 int port = WEBCFACE_DEFAULT_PORT);

    WEBCFACE_DLL explicit Client(const std::string &name,
                                 const std::string &host, int port,
                                 std::shared_ptr<Internal::ClientData> data);

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
    WEBCFACE_DLL std::vector<Member> members();
    //! Memberが追加された時のイベント
    /*! コールバックの型は void(Member)
     *
     * \sa member(), members()
     */
    WEBCFACE_DLL EventTarget<Member, int> onMemberEntry();

    //! これ以降セットするFuncのデフォルトのFuncWrapperをセットする。(初期状態はnullptr)
    /*!
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     */
    WEBCFACE_DLL void setDefaultRunCond(FuncWrapperType wrapper);

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
    /*!
     * \sa logger(), loggerStreamBuf(), loggerOStream()
     */
    WEBCFACE_DLL std::shared_ptr<LoggerSink> loggerSink();
    //! webcfaceとstderr_sinkに出力するlogger
    /*!
     * \sa loggerSink(), loggerStreamBuf(), loggerOStream()
     */
    WEBCFACE_DLL std::shared_ptr<spdlog::logger> logger();

    //! webcfaceに出力するstreambuf
    /*! levelは常にinfoになる。
     * std::flushのタイミングとは無関係に、1つの改行ごとに1つのログになる
     *
     * \sa loggerSink(), logger(), loggerOStream()
     */
    LoggerBuf *loggerStreamBuf() { return &logger_buf; }
    //! webcfaceに出力するostream
    /*!
     * \sa loggerSink(), logger(), loggerStreamBuf()
     */
    std::ostream &loggerOStream() { return logger_os; }

    //! WebCFaceサーバーのバージョン情報
    WEBCFACE_DLL std::string serverVersion() const;
    //! WebCFaceサーバーの識別情報
    /*!
     * \return webcface付属のサーバーであれば通常は "webcface" が返る
     * \sa serverVersion()
     */
    WEBCFACE_DLL std::string serverName() const;
};

} // namespace WebCFace

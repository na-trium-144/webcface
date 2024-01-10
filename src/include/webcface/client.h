#pragma once
#include <string>
#include <memory>
#include <vector>
#include <spdlog/logger.h>
#include "member.h"
#include "event_target.h"
#include "func.h"
#include "logger.h"
#include "common/def.h"

namespace WEBCFACE_NS {
/*!
 * \brief サーバーに接続するクライアント。
 *
 */
class Client : public Member {
    std::shared_ptr<Internal::ClientData> data;

  public:
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;

    /*!
     * \brief 名前を指定せずサーバーに接続する
     *
     * サーバーのホストとポートはlocalhost:7530になる
     *
     */
    Client() : Client("") {}
    /*!
     * \brief 名前を指定しサーバーに接続する
     *
     * サーバーのホストとポートを省略した場合localhost:7530になる
     *
     * \arg name 名前
     * \arg host サーバーのアドレス
     * \arg port サーバーのポート
     *
     */
    WEBCFACE_DLL explicit Client(const std::string &name,
                                 const std::string &host = "127.0.0.1",
                                 int port = WEBCFACE_DEFAULT_PORT);

    WEBCFACE_DLL explicit Client(const std::string &name,
                                 std::shared_ptr<Internal::ClientData> data);

    /*!
     * \brief サーバーに接続できているときtrueを返す
     *
     */
    WEBCFACE_DLL bool connected() const;
    /*!
     * \brief 接続を切りClientを破棄
     *
     */
    WEBCFACE_DLL ~Client();
    /*!
     * \brief 接続を切り、今後再接続しない
     *
     */
    WEBCFACE_DLL void close();

    /*!
     * \brief (ver1.2で追加) サーバーに接続を開始する。
     *
     */
    WEBCFACE_DLL void start();
    /*!
     * \brief (ver1.2で追加) サーバーに接続が成功するまで待機する。
     *
     * 接続していない場合、start()を呼び出す。
     * \sa start()
     *
     */
    WEBCFACE_DLL void waitConnection();

    /*!
     * \brief 送信用にセットしたデータをすべて送信キューに入れる。
     *
     * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
     *
     * ver1.2以降: サーバーに接続していない場合、start()を呼び出す。
     * \sa start()
     *
     */
    WEBCFACE_DLL void sync();

    /*!
     * \brief 他のmemberにアクセスする
     *
     * \sa members(), onMemberEntry()
     *
     */
    Member member(const std::string &name) { return Member{data, name}; }
    /*!
     * \brief サーバーに接続されている他のmemberのリストを得る。
     *
     * 自分自身と、無名のmemberを除く。
     * \sa member(), onMemberEntry()
     *
     */
    WEBCFACE_DLL std::vector<Member> members();
    /*!
     * \brief Memberが追加された時のイベント
     *
     * コールバックの型は void(Member)
     *
     * \sa member(), members()
     */
    WEBCFACE_DLL EventTarget<Member, int> onMemberEntry();

    /*!
     * \brief
     * これ以降セットするFuncのデフォルトのFuncWrapperをセットする。(初期状態はnullptr)
     *
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     *
     */
    WEBCFACE_DLL void setDefaultRunCond(FuncWrapperType wrapper);

    /*!
     * \brief デフォルトのFuncWrapperを nullptr にする
     *
     */
    void setDefaultRunCondNone() { setDefaultRunCond(nullptr); }
    /*!
     * \brief デフォルトのFuncWrapperを runCondOnSync() にする
     *
     */
    void setDefaultRunCondOnSync() {
        setDefaultRunCond(FuncWrapper::runCondOnSync(data));
    }
    /*!
     * \brief デフォルトのFuncWrapperを runCondScopeGuard() にする
     *
     */
    template <typename ScopeGuard>
    void setDefaultRunCondScopeGuard() {
        setDefaultRunCond(FuncWrapper::runCondScopeGuard<ScopeGuard>());
    }

    /*!
     * \brief webcfaceに出力するsink
     *
     * (v1.0.1で logger_sink から名前変更)
     * \sa logger(), loggerStreamBuf(), loggerOStream()
     *
     */
    WEBCFACE_DLL std::shared_ptr<LoggerSink> loggerSink();
    /*!
     * \brief webcfaceとstderr_sinkに出力するlogger
     *
     * 初期状態では logger()->sinks() = { loggerSink(), stderr_color_sink_mt }
     * となっている
     *
     * \sa loggerSink(), loggerStreamBuf(), loggerOStream()
     *
     */
    WEBCFACE_DLL std::shared_ptr<spdlog::logger> logger();

    /*!
     * \brief webcfaceに出力するstreambuf
     *
     * (v1.0.1で logger_streambuf から名前変更)
     *
     * levelは常にinfoになる。
     * std::flushのタイミングとは無関係に、1つの改行ごとに1つのログになる
     *
     * \sa loggerSink(), logger(), loggerOStream()
     *
     */
    WEBCFACE_DLL LoggerBuf *loggerStreamBuf();
    /*!
     * \brief webcfaceに出力するostream
     *
     * (v1.0.1で logger_ostream から名前変更)
     * \sa loggerSink(), logger(), loggerStreamBuf()
     *
     */
    WEBCFACE_DLL std::ostream &loggerOStream();

    /*!
     * \brief WebCFaceサーバーのバージョン情報
     *
     */
    WEBCFACE_DLL std::string serverVersion() const;
    /*!
     * \brief WebCFaceサーバーの識別情報
     *
     * \return webcface付属のサーバーであれば通常は "webcface" が返る
     * \sa serverVersion()
     *
     */
    WEBCFACE_DLL std::string serverName() const;
};

} // namespace WEBCFACE_NS

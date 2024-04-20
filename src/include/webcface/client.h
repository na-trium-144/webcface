#pragma once
#include <string>
#include <memory>
#include <vector>
#include "member.h"
#include "encoding.h"
#include "event_target.h"
#include "common/def.h"

namespace spdlog {
class logger;
}

WEBCFACE_NS_BEGIN

class FuncListener;
class LoggerSink;
class LoggerBuf;

/*!
 * \brief サーバーに接続するクライアント。
 *
 */
class WEBCFACE_DLL Client : public Member {
    std::shared_ptr<Internal::ClientData> data;

  public:
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;

    /*!
     * \brief 名前を指定せずサーバーに接続する
     *
     * サーバーのホストとポートは127.0.0.1:7530になる
     *
     */
    Client() : Client("") {}
    /*!
     * \brief 名前を指定しサーバーに接続する
     *
     * サーバーのホストとポートを省略した場合127.0.0.1:7530になる
     *
     * \arg name 名前
     * \arg host サーバーのアドレス
     * \arg port サーバーのポート
     *
     */
    explicit Client(std::string_view name, std::string_view host = "127.0.0.1",
                    int port = WEBCFACE_DEFAULT_PORT);
    /*!
     * \brief 名前を指定しサーバーに接続する (wstring)
     * \since ver1.11
     *
     * \arg name 名前
     * \arg host サーバーのアドレス
     * \arg port サーバーのポート
     *
     */
    explicit Client(std::wstring_view name,
                    std::wstring_view host = L"127.0.0.1",
                    int port = WEBCFACE_DEFAULT_PORT);

    /*!
     * テストで使用
     *
     */
    explicit Client(std::string_view name,
                    std::shared_ptr<Internal::ClientData> data);

    /*!
     * \brief サーバーに接続できているときtrueを返す
     *
     */
    bool connected() const;
    /*!
     * \brief 接続を切りClientを破棄
     *
     */
    ~Client();
    /*!
     * \brief 接続を切り、今後再接続しない
     *
     */
    void close();

    /*!
     * \brief (ver1.2で追加) サーバーに接続を開始する。
     *
     */
    void start();
    /*!
     * \brief (ver1.2で追加) サーバーに接続が成功するまで待機する。
     *
     * 接続していない場合、start()を呼び出す。
     * \sa start()
     *
     */
    void waitConnection();

    /*!
     * \brief 送信用にセットしたデータをすべて送信キューに入れる。
     *
     * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
     *
     * ver1.2以降: サーバーに接続していない場合、start()を呼び出す。
     * \sa start()
     *
     */
    void sync();

    Member member(std::u8string_view name) {
        if (name.empty()) {
            return *this;
        } else {
            return Member{data, name};
        }
    }
    /*!
     * \brief 他のmemberにアクセスする
     *
     * (ver1.7から)nameが空の場合 *this を返す
     * \sa members(), onMemberEntry()
     *
     */
    Member member(std::string_view name) {
        return member(Encoding::initName(name));
    }
    /*!
     * \brief 他のmemberにアクセスする (wstring)
     * \since ver1.11
     *
     * nameが空の場合 *this を返す
     * \sa members(), onMemberEntry()
     *
     */
    Member member(std::wstring_view name) {
        return member(Encoding::initNameW(name));
    }
    /*!
     * \brief サーバーに接続されている他のmemberのリストを得る。
     *
     * 自分自身と、無名のmemberを除く。
     * \sa member(), onMemberEntry()
     *
     */
    std::vector<Member> members();
    /*!
     * \brief Memberが追加された時のイベント
     *
     * コールバックの型は void(Member)
     *
     * \sa member(), members()
     */
    EventTarget<Member, int> onMemberEntry();

    FuncListener funcListener(std::u8string_view field) const;
    /*!
     * \brief FuncListenerを作成する
     *
     */
    FuncListener funcListener(std::string_view field) const {
        return funcListener(Encoding::initName(field));
    }
    /*!
     * \brief FuncListenerを作成する (wstring)
     * \since ver1.11
     */
    FuncListener funcListener(const std::wstring &field) const {
        return funcListener(Encoding::initNameW(field));
    }

    /*!
     * \brief
     * これ以降セットするFuncのデフォルトのFuncWrapperをセットする。(初期状態はnullptr)
     *
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     *
     */
    void setDefaultRunCond(FuncWrapperType wrapper);

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
    std::shared_ptr<LoggerSink> loggerSink();
    /*!
     * \brief webcfaceとstderr_sinkに出力するlogger
     *
     * 初期状態では logger()->sinks() = { loggerSink(), stderr_color_sink_mt }
     * となっている
     *
     * \sa loggerSink(), loggerStreamBuf(), loggerOStream()
     *
     */
    std::shared_ptr<spdlog::logger> logger();

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
    LoggerBuf *loggerStreamBuf();
    /*!
     * \brief webcfaceに出力するostream
     *
     * (v1.0.1で logger_ostream から名前変更)
     * \sa loggerSink(), logger(), loggerStreamBuf()
     *
     */
    std::ostream &loggerOStream();

    /*!
     * \brief WebCFaceサーバーのバージョン情報
     *
     */
    std::string serverVersion() const;
    /*!
     * \brief WebCFaceサーバーの識別情報
     *
     * \return webcface付属のサーバーであれば通常は "webcface" が返る
     * \sa serverVersion()
     *
     */
    std::string serverName() const;
};

WEBCFACE_NS_END

#pragma once
#include <string>
#include <memory>
#include <vector>
#include "member.h"
#include "event_target.h"
#include <webcface/common/def.h>

namespace spdlog {
class logger;
}

WEBCFACE_NS_BEGIN

class LoggerSink;
template <typename CharT>
class BasicLoggerBuf;
using LoggerBuf = BasicLoggerBuf<char>;
using LoggerBufW = BasicLoggerBuf<wchar_t>;

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
     * サーバーのホストとポートはlocalhost:7530になる
     *
     */
    Client() : Client("") {}
    /*!
     * \brief 名前を指定しサーバーに接続する
     *
     * サーバーのホストとポートを省略した場合 127.0.0.1:7530 になる
     *
     * \arg name 名前
     * \arg host サーバーのアドレス
     * \arg port サーバーのポート
     *
     */
    explicit Client(const std::string &name,
                    const std::string &host = "127.0.0.1",
                    int port = WEBCFACE_DEFAULT_PORT)
        : Client(SharedString(name), SharedString(host), port) {}
    /*!
     * \brief 名前を指定しサーバーに接続する (wstring)
     * \since ver1.12
     *
     * サーバーのホストとポートを省略した場合 127.0.0.1:7530 になる
     *
     * \arg name 名前
     * \arg host サーバーのアドレス
     * \arg port サーバーのポート
     *
     */
    explicit Client(const std::wstring &name,
                    const std::wstring &host = L"127.0.0.1",
                    int port = WEBCFACE_DEFAULT_PORT)
        : Client(SharedString(name), SharedString(host), port) {}

    explicit Client(const SharedString &name, const SharedString &host,
                    int port);
    explicit Client(const SharedString &name,
                    const std::shared_ptr<Internal::ClientData> &data);

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
     * \brief 通信が切断されたときに自動で再試行するかどうかを設定する。
     * \since ver1.11.1
     *
     * デフォルトはtrue
     *
     */
    void autoReconnect(bool enabled);
    /*!
     * \brief 通信が切断されたときに自動で再試行するかどうかを取得する。
     * \since ver1.11.1
     */
    bool autoReconnect() const;

    /*!
     * \brief サーバーへの接続を開始する。
     * \since ver1.2
     */
    void start();
    /*!
     * \brief サーバーへの接続を開始し、成功するまで待機する。
     * \since ver1.2
     * ver1.11.1以降: autoReconnect が false
     * の場合は1回目の接続のみ待機し、失敗しても再接続せずreturnする。
     *
     * \sa start()
     */
    void waitConnection();

    /*!
     * \brief 送信用にセットしたデータをすべて送信キューに入れる。
     *
     * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
     *
     * ver1.2以降: サーバーに接続していない場合、start()を呼び出す。
     *
     * \sa start()
     */
    void sync();

  protected:
    Member member(const SharedString &name) const {
        if (name.empty()) {
            return *this;
        } else {
            return Member{data, name};
        }
    }

  public:
    /*!
     * \brief 他のmemberにアクセスする
     *
     * (ver1.7から)nameが空の場合 *this を返す
     *
     * \sa members(), onMemberEntry()
     */
    Member member(std::string_view name) const {
        return member(SharedString(name));
    }
    /*!
     * \brief 他のmemberにアクセスする (wstring)
     * \since ver1.12
     * nameが空の場合 *this を返す
     *
     * \sa members(), onMemberEntry()
     */
    Member member(std::wstring_view name) const {
        return member(SharedString(name));
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
    EventTarget<Member> onMemberEntry();

    /*!
     * \brief
     * これ以降セットするFuncのデフォルトのFuncWrapperをセットする。(初期状態はnullptr)
     *
     * Funcの実行時にFuncWrapperを通すことで条件を満たすまでブロックしたりする。
     * FuncWrapperがnullptrなら何もせずsetした関数を実行する
     *
     */
    void setDefaultRunCond(const FuncWrapperType &wrapper);

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
     * * ver1.0.1で logger_sink から名前変更
     * * ver1.12から、 Encoding::usingUTF8()
     * がfalseの場合ログに書き込まれたstringはutf-8に変換されてからwebcfaceに送られる
     *
     * \sa logger(), loggerStreamBuf(), loggerOStream()
     */
    std::shared_ptr<LoggerSink> loggerSink();
    /*!
     * \brief webcfaceとstderr_sinkに出力するlogger
     *
     * * 初期状態では logger()->sinks() = { loggerSink(), stderr_color_sink_mt }
     * となっているためこれを利用すると簡単にログ出力が可能だが、
     * 必ずしもこれを使う必要はない
     * (別のloggerのsinkに loggerSink() を追加するのでもよい)
     *
     * \sa loggerSink(), loggerStreamBuf(), loggerOStream()
     */
    std::shared_ptr<spdlog::logger> logger();

    /*!
     * \brief webcfaceに出力するstreambuf
     *
     * * (ver1.11.xまで:
     * std::ostreamの出力先として使用すると、logger()に送られる。
     * すなわち、loggerSink (webcfaceに送られる) と stderr_color_sink_mt
     * (標準エラー出力に送られる) に出力されることになる。)
     * * ver1.12〜: std::ostreamの出力先として使用すると、改行が入力されるたびに
     * webcfaceに送られると同時に std::cerr にも送られる。
     * * levelは常にinfoになる。
     * * std::flushのタイミングとは無関係に、1つの改行ごとに1つのログになる
     * * ver1.0.1で logger_streambuf から名前変更
     *
     * \sa loggerSink(), logger(), loggerOStream()
     */
    LoggerBuf *loggerStreamBuf();
    /*!
     * \brief webcfaceに出力するostream
     *
     * * (ver1.11.xまで: 出力先が loggerStreamBuf() に設定されているostream。
     * すなわち、loggerSink (webcfaceに送られる) と stderr_color_sink_mt
     * (標準エラー出力に送られる) に出力されることになる。)
     * * ver1.12〜: std::ostreamの出力先として使用すると、改行が入力されるたびに
     * webcfaceに送られると同時に std::cerr にも送られる。
     * * ver1.0.1で logger_ostream から名前変更
     *
     * \sa loggerSink(), logger(), loggerStreamBuf()
     */
    std::ostream &loggerOStream();
    /*!
     * \brief webcfaceに出力するwstreambuf
     * \since ver1.12
     * \sa loggerStreamBuf
     */
    LoggerBufW *loggerWStreamBuf();
    /*!
     * \brief webcfaceに出力するwostream
     * \since ver1.12
     * \sa loggerOStream
     */
    std::wostream &loggerWOStream();

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

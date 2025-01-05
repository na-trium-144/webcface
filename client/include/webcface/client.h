#pragma once
#include <string>
#include <memory>
#include <vector>
#include "member.h"
#include "webcface/c_wcf/client.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief サーバーに接続するクライアント。
 *
 */
class WEBCFACE_DLL Client : public Member {
    wcfClient *data;
    std::function<void WEBCFACE_CALL_FP(Member)> on_member_entry_;

  public:
    Client(const Client &) = delete;
    const Client &operator=(const Client &) = delete;

    /*!
     * \brief 名前を指定せずサーバーに接続する
     *
     * サーバーのホストとポートはlocalhost:7530になる
     *
     */
    Client() : data(wcfInitDefault("")) {}
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
        : data(wcfInit(name.c_str(), host.c_str(), port)) {}
    /*!
     * \brief 名前を指定しサーバーに接続する (wstring)
     * \since ver2.0
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
        : data(wcfInitW(name.c_str(), host.c_str(), port)) {}

    /*!
     * \brief サーバーに接続できているときtrueを返す
     *
     */
    bool connected() const { return wcfIsConnected(data); }
    /*!
     * \brief 接続を切りClientを破棄
     *
     */
    ~Client() { wcfClose(data); }
    /*!
     * \brief 接続を切り、今後再接続しない
     *
     */
    const Client &close() const {
        wcfClose(data);
        return *this;
    }

    /*!
     * \brief 通信が切断されたときに自動で再試行するかどうかを設定する。
     * \since ver1.11.1
     *
     * デフォルトはtrue
     *
     * \sa start(), waitConnection()
     */
    const Client &autoReconnect(bool enabled) const {
        wcfAutoReconnect(data, enabled);
        return *this
    }
    /*!
     * \brief 通信が切断されたときに自動で再試行するかどうかを取得する。
     * \since ver1.11.1
     */
    bool autoReconnect() const { return wcfAutoReconnectEnabled(data); }

    /*!
     * \brief サーバーへの接続を別スレッドで開始する。
     * \since ver1.2
     * \sa waitConnection(), autoReconnect()
     */
    const Client &start() const {
        wcfStart(data);
        return *this;
    }
    /*!
     * \brief サーバーへの接続を別スレッドで開始し、成功するまで待機する。
     * \since ver1.2
     *
     * * ver1.11.1以降: autoReconnect が false
     * の場合は1回目の接続のみ待機し、失敗しても再接続せずreturnする。
     * * ver2.0以降: 接続だけでなくentryの受信や初期化が完了するまで待機する。
     * loopSync() と同様、このスレッドで受信処理
     * (onEntry コールバックの呼び出しなど) が行われる。
     *
     * \sa start(), autoReconnect()
     */
    const Client &waitConnection() const {
        wcfWaitConnection(data);
        return *this;
    }

    /*!
     * \brief
     * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する。
     *
     * * 実際に送信をするのは別スレッドであり、この関数はブロックしない。
     * * ver1.2以降: サーバーに接続していない場合、start()を呼び出す。
     * * ver2.0以降: データを受信した場合、各種コールバック(onEntry, onChange,
     * Func::run()など)をこのスレッドで呼び出し、
     * それがすべて完了するまでこの関数はブロックされる。
     *   * データをまだ何も受信していない場合やサーバーに接続していない場合は、
     * 即座にreturnする。
     *
     * \sa start(), loopSyncFor(), loopSyncUntil(), loopSync()
     */
    const Client &sync() const {
        wcfSync(data);
        return *this;
    }
    /*!
     * \brief
     * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する。
     * \since ver2.0
     *
     * * sync()と同じだが、データを受信してもしなくても
     * timeout 経過するまでは繰り返しsync()を再試行する。
     * timeout=0 または負の値なら再試行せず即座にreturnする。(sync()と同じ)
     * * autoReconnectがfalseでサーバーに接続できてない場合はreturnする。
     * (deadlock回避)
     *
     * \sa sync(), loopSyncUntil(), loopSync()
     */
    const Client &loopSyncFor(std::chrono::microseconds timeout) const {
        wcfLoopSyncFor(data, timeout.count());
        return *this;
    }
    /*!
     * \brief
     * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する。
     * \since ver2.0
     *
     * loopSyncFor() と同じだが、timeoutを絶対時間で指定
     *
     * \sa sync(), loopSyncFor(), loopSync()
     */
    template <typename Clock, typename Duration>
    const Client &
    loopSyncUntil(std::chrono::time_point<Clock, Duration> timeout) const {
        return loopSyncFor(
            std::chrono::duration_cast<std::chrono::microseconds>(
                timeout - Clock::now()));
    }
    /*!
     * \brief
     * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する。
     * \since ver2.0
     *
     * * loopSyncFor()と同じだが、close()されるまで無制限にsync()を再試行する。
     * * autoReconnectがfalseでサーバーに接続できてない場合はreturnする。
     * (deadlock回避)
     *
     * \sa sync(), loopSyncFor(), loopSyncUntil()
     */
    const Client &loopSync() const {
        wcfLoopSync(data);
        return *this;
    }

    /*!
     * \brief 他のmemberにアクセスする
     *
     * (ver1.7から)nameが空の場合 *this を返す
     *
     * \sa members(), onMemberEntry()
     */
    Member member(std::string_view name) const {
        if (name.empty()) {
            return *this;
        } else {
            return Member{data, SharedString::encode(name)};
        }
    }
    /*!
     * \brief 他のmemberにアクセスする (wstring)
     * \since ver2.0
     * nameが空の場合 *this を返す
     *
     * \sa members(), onMemberEntry()
     */
    Member member(std::wstring_view name) const {
        if (name.empty()) {
            return *this;
        } else {
            return Member{data, SharedString::encode(name)};
        }
    }
    /*!
     * \brief サーバーに接続されている他のmemberのリストを得る。
     *
     * 自分自身と、無名のmemberを除く。
     * \sa member(), onMemberEntry()
     */
    std::vector<Member> members() const {
        int members_num;
        wcfMemberList(data, nullptr, 0, &members_num);
        std::vector<const char *> member_names(members_num);
        wcfMemberList(data, member_names.data(), members_num, nullptr);
        std::vector<Member> members;
        for (auto name : member_names) {
            members.push_back(member(name));
        }
        return members;
    }
    /*!
     * \brief Memberが追加された時のイベント
     *
     * コールバックの型は void(Member)
     *
     * \sa member(), members()
     */
    const Client &
    onMemberEntry(std::function<void WEBCFACE_CALL_FP(Member)> callback) {
        on_member_entry_ = std::move(callback);
        wcfMemberEntryEvent(
            data,
            [](const char *name, void *this_v) {
                auto this_ = static_cast<Client *>(this_v);
                this_->on_member_entry_(
                    Member{this_->data, SharedString::encode(name)});
            },
            this);
        return *this;
    }

    /*!
     * \brief webcfaceに出力するstreambuf
     *
     * * (ver1.11.xまで:
     * std::ostreamの出力先として使用すると、logger()に送られる。
     * すなわち、loggerSink (webcfaceに送られる) と stderr_color_sink_mt
     * (標準エラー出力に送られる) に出力されることになる。)
     * * ver2.0〜: std::ostreamの出力先として使用すると、改行が入力されるたびに
     * webcfaceに送られると同時に stderr にも送られる。
     * * levelは常にinfoになる。
     * * std::flushのタイミングとは無関係に、1つの改行ごとに1つのログになる
     * * ver1.0.1で logger_streambuf から名前変更
     *
     * \sa loggerOStream()
     */
    std::streambuf *loggerStreamBuf() const;
    /*!
     * \brief webcfaceに出力するstreambuf
     * \since ver2.4
     *
     * * nameを省略した場合 "default" になる。
     *
     */
    std::streambuf *loggerStreamBuf(std::string_view name) const;
    /*!
     * \brief webcfaceに出力するostream
     *
     * * (ver1.11.xまで: 出力先が loggerStreamBuf() に設定されているostream。
     * すなわち、loggerSink (webcfaceに送られる) と stderr_color_sink_mt
     * (標準エラー出力に送られる) に出力されることになる。)
     * * ver2.0〜: std::ostreamの出力先として使用すると、改行が入力されるたびに
     * webcfaceに送られると同時に stderr にも送られる。
     * * ver1.0.1で logger_ostream から名前変更
     *
     * \sa loggerStreamBuf()
     */
    std::ostream &loggerOStream() const;
    /*!
     * \brief webcfaceに出力するostream
     * \since ver2.4
     *
     * * nameを省略した場合 "default" になる。
     *
     */
    std::ostream &loggerOStream(std::string_view name) const;
    /*!
     * \brief webcfaceに出力するwstreambuf
     * \since ver2.0
     * \sa loggerStreamBuf
     */
    std::wstreambuf *loggerWStreamBuf() const;
    /*!
     * \brief webcfaceに出力するwstreambuf
     * \since ver2.4
     *
     * * nameを省略した場合 "default" になる。
     *
     */
    std::wstreambuf *loggerWStreamBuf(std::wstring_view name) const;
    /*!
     * \brief webcfaceに出力するwostream
     * \since ver2.0
     * \sa loggerOStream
     */
    std::wostream &loggerWOStream() const;
    /*!
     * \brief webcfaceに出力するwostream
     * \since ver2.4
     *
     * * nameを省略した場合 "default" になる。
     *
     */
    std::wostream &loggerWOStream(std::wstring_view name) const;

    /*!
     * \brief WebCFaceサーバーのバージョン情報
     *
     */
    std::string_view serverVersion() const{
        return wcfServerVersion(data);
    }
    /*!
     * \brief WebCFaceサーバーの識別情報
     *
     * \return webcface付属のサーバーであれば通常は "webcface" が返る
     * \sa serverVersion()
     *
     */
    std::string_view serverName() const{
        return wcfServerName(data);
    }
    /*!
     * \brief サーバーのホスト名
     * \since ver2.0
     */
    std::string_view serverHostName() const{
        return wcfServerHostName(data);
    }
};

WEBCFACE_NS_END

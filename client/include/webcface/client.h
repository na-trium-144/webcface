#pragma once
#include <string>
#include <memory>
#include <vector>
#include "member.h"
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
    std::shared_ptr<internal::ClientData> data;

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
        : Client(SharedString::encode(name), SharedString::encode(host), port) {
    }
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
        : Client(SharedString::encode(name), SharedString::encode(host), port) {
    }

    explicit Client(const SharedString &name, const SharedString &host,
                    int port);
    explicit Client(const SharedString &name,
                    const std::shared_ptr<internal::ClientData> &data);

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
     * \sa start(), waitConnection()
     */
    void autoReconnect(bool enabled);
    /*!
     * \brief 通信が切断されたときに自動で再試行するかどうかを取得する。
     * \since ver1.11.1
     */
    bool autoReconnect() const;

    /*!
     * \brief サーバーへの接続を別スレッドで開始する。
     * \since ver1.2
     * \sa waitConnection(), autoReconnect()
     */
    void start();
    /*!
     * \brief サーバーへの接続を別スレッドで開始し、成功するまで待機する。
     * \since ver1.2
     *
     * * ver1.11.1以降: autoReconnect が false
     * の場合は1回目の接続のみ待機し、失敗しても再接続せずreturnする。
     * * ver2.0以降: 接続だけでなくentryの受信や初期化が完了するまで待機する。
     * waitSync() と同様、このスレッドで受信処理
     * (onEntry コールバックの呼び出しなど) が行われる。
     *
     * \sa start(), autoReconnect()
     */
    void waitConnection();

  private:
    void syncImpl(std::optional<std::chrono::microseconds> timeout);

  public:
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
     * \sa start(), waitSyncFor(), waitSyncUntil(), waitSync()
     */
    void sync() { syncImpl(std::chrono::microseconds(0)); }
    /*!
     * \brief
     * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する。
     * \since ver2.0
     *
     * * sync()と同じだが、何も受信できなければ
     * timeout 経過後に再試行してreturnする。
     * timeout=0 または負の値なら再試行せず即座にreturnする。(sync()と同じ)
     * * autoReconnectがfalseでサーバーに接続できてない場合はreturnする。
     * (deadlock回避)
     * * timeoutが100μs以上の場合、100μsおきに繰り返し再試行し、timeout経過後return
     *
     * \sa sync(), waitSyncUntil(), waitSync()
     */
    void waitSyncFor(std::chrono::microseconds timeout) { syncImpl(timeout); }
    /*!
     * \brief
     * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する。
     * \since ver2.0
     *
     * waitSyncFor() と同じだが、timeoutを絶対時間で指定
     *
     * \sa sync(), waitSyncFor(), waitSync()
     */
    template <typename Clock, typename Duration>
    void waitSyncUntil(std::chrono::time_point<Clock, Duration> timeout) {
        syncImpl(std::chrono::duration_cast<std::chrono::microseconds>(
            timeout - Clock::now()));
    }
    /*!
     * \brief
     * 送信用にセットしたデータをすべて送信キューに入れ、受信したデータを処理する。
     * \since ver2.0
     *
     * * waitSyncFor()と同じだが、何か受信するまで無制限に待機する
     * * autoReconnectがfalseでサーバーに接続できてない場合はreturnする。
     * (deadlock回避)
     *
     * \sa sync(), waitSyncFor(), waitSyncUntil()
     */
    void waitSync() { syncImpl(std::nullopt); }

    // /*!
    //  * \brief 別スレッドでsync()を自動的に呼び出す間隔を設定する。
    //  * \since ver2.0
    //  *
    //  * * start() や waitConnection() より前に設定する必要がある。
    //  * * autoSyncが有効の場合、別スレッドで一定間隔(100μs)ごとにsync()が呼び出され、
    //  * 各種コールバック (onEntry, onChange, Func::run()など)
    //  * も別のスレッドで呼ばれることになる
    //  * (そのためmutexなどを適切に設定すること)
    //  * * デフォルトでは無効なので、手動でsync()などを呼び出す必要がある
    //  *
    //  * \param enabled trueにすると自動でsync()が呼び出されるようになる
    //  * \sa sync(), waitSyncFor(), waitSyncUntil(), waitSync()
    //  */
    // void autoSync(bool enabled);

  private:
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
        return member(SharedString::encode(name));
    }
    /*!
     * \brief 他のmemberにアクセスする (wstring)
     * \since ver2.0
     * nameが空の場合 *this を返す
     *
     * \sa members(), onMemberEntry()
     */
    Member member(std::wstring_view name) const {
        return member(SharedString::encode(name));
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
    Client &
    onMemberEntry(std::function<void WEBCFACE_CALL_FP(Member)> callback);

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
    std::streambuf *loggerStreamBuf();
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
    std::ostream &loggerOStream();
    /*!
     * \brief webcfaceに出力するwstreambuf
     * \since ver2.0
     * \sa loggerStreamBuf
     */
    std::wstreambuf *loggerWStreamBuf();
    /*!
     * \brief webcfaceに出力するwostream
     * \since ver2.0
     * \sa loggerOStream
     */
    std::wostream &loggerWOStream();

    /*!
     * \brief WebCFaceサーバーのバージョン情報
     *
     */
    const std::string &serverVersion() const;
    /*!
     * \brief WebCFaceサーバーの識別情報
     *
     * \return webcface付属のサーバーであれば通常は "webcface" が返る
     * \sa serverVersion()
     *
     */
    const std::string &serverName() const;
    /*!
     * \brief サーバーのホスト名
     * \since ver2.0
     */
    const std::string &serverHostName() const;
};

WEBCFACE_NS_END

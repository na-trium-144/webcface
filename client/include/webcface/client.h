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
     * * サーバーのホストとポートを省略した場合 127.0.0.1:7530 になる
     * * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     *
     * \arg name 名前
     * \arg host サーバーのアドレス
     * \arg port サーバーのポート
     *
     */
    explicit Client(StringInitializer name, StringInitializer host = "127.0.0.1",
                    int port = WEBCFACE_DEFAULT_PORT)
        : Client(static_cast<SharedString &>(name),
                 static_cast<SharedString &>(host), port) {}

    explicit Client(const SharedString &name, const SharedString &host,
                    int port);
    explicit Client(const SharedString &name,
                    const std::shared_ptr<internal::ClientData> &data);

    /*!
     * \brief サーバーに接続できているときtrueを返す
     *
     * * (ver2.9〜) Member::connected() と同じ。
     */
    bool connected() const;

    /*!
     * \brief 切断したときに呼び出されるコールバックを設定
     * \since ver2.9
     * \param callback 引数をとらない関数
     *
     * * 通信が切断されて以降で最初のsync()の中で呼ばれる
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Client &onDisconnect(F callback) const {
        this->Member::onDisconnect(
            [callback = std::move(callback)](const auto &) { callback(); });
        return *this;
    }
    /*!
     * \brief サーバーに接続したときに呼び出されるコールバックを設定
     * \since ver2.9
     * \param callback 引数をとらない関数
     *
     * * sync() または waitConnection() の中で、
     * 各種Entryイベントをすべて呼び終わったあと、
     * waitConnection() がreturnするまえに呼ばれる。
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Client &onConnect(F callback) const {
        this->Member::onConnect(
            [callback = std::move(callback)](const auto &) { callback(); });
        return *this;
    }

    /*!
     * \brief 接続を切りClientを破棄
     *
     */
    ~Client();
    /*!
     * \brief 接続を切り、今後再接続しない
     *
     */
    const Client &close() const;

    /*!
     * \brief 通信が切断されたときに自動で再試行するかどうかを設定する。
     * \since ver1.11.1
     *
     * デフォルトはtrue
     *
     * \sa start(), waitConnection()
     */
    const Client &autoReconnect(bool enabled) const;
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
    const Client &start() const;
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
    const Client &waitConnection() const;

  private:
    const Client &
    syncImpl(std::optional<std::chrono::microseconds> timeout) const;

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
     * \sa start(), loopSyncFor(), loopSyncUntil(), loopSync()
     */
    const Client &sync() const {
        return syncImpl(std::chrono::microseconds(0));
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
        return syncImpl(timeout);
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
        return syncImpl(std::chrono::duration_cast<std::chrono::microseconds>(
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
    const Client &loopSync() const { return syncImpl(std::nullopt); }

    // /*!
    //  * \brief 別スレッドでsync()を自動的に呼び出す間隔を設定する。
    //  * \since ver2.0
    //  *
    //  * * start() や waitConnection() より前に設定する必要がある。
    //  * *
    //  autoSyncが有効の場合、別スレッドで一定間隔(100μs)ごとにsync()が呼び出され、
    //  * 各種コールバック (onEntry, onChange, Func::run()など)
    //  * も別のスレッドで呼ばれることになる
    //  * (そのためmutexなどを適切に設定すること)
    //  * * デフォルトでは無効なので、手動でsync()などを呼び出す必要がある
    //  *
    //  * \param enabled trueにすると自動でsync()が呼び出されるようになる
    //  * \sa sync(), loopSyncFor(), loopSyncUntil(), loopSync()
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
     * * (ver1.7から)nameが空の場合 *this を返す
     * * ver2.0〜 wstring対応, ver2.10〜 StringInitializer 型で置き換え
     *
     * \sa members(), onMemberEntry()
     */
    Member member(StringInitializer name) const {
        return member(static_cast<SharedString &>(name));
    }
    /*!
     * \brief サーバーに接続されている他のmemberのリストを得る。
     *
     * 自分自身と、無名のmemberを除く。
     * \sa member(), onMemberEntry()
     */
    std::vector<Member> members();
    /*!
     * \brief サーバーに接続されている他のmemberのリストを得る。
     * \since ver2.0.2 (constつけ忘れ)
     *
     * 自分自身と、無名のmemberを除く。
     * \sa member(), onMemberEntry()
     */
    std::vector<Member> members() const;
    /*!
     * \brief Memberが追加された時のイベント
     *
     * コールバックの型は void(Member)
     *
     * \sa member(), members()
     */
    const Client &
    onMemberEntry(std::function<void WEBCFACE_CALL_FP(Member)> callback) const;

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
     * * ver2.10〜 StringInitializer 型に変更
     *
     */
    std::streambuf *loggerStreamBuf(const StringInitializer &name) const;
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
     * * ver2.10〜 StringInitializer 型に変更
     *
     */
    std::ostream &loggerOStream(const StringInitializer &name) const;
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
     * * ver2.10〜 StringInitializer 型に変更
     *
     */
    std::wstreambuf *loggerWStreamBuf(const StringInitializer &name) const;
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
     * * ver2.10〜 StringInitializer 型に変更
     *
     */
    std::wostream &loggerWOStream(const StringInitializer &name) const;

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

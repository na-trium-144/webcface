#pragma once
#include <functional>
#include <optional>
#include <vector>
#include <chrono>
#include "field.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace message {
struct LogLine;
}

namespace level {
enum LogLevelEnum {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    warning = 3,
    error = 4,
    critical = 5,
};
}

struct WEBCFACE_DLL LogLineData {
    int level_ = 0;
    std::chrono::system_clock::time_point time_;
    SharedString message_;

    LogLineData() = default;
    LogLineData(int level, std::chrono::system_clock::time_point time,
                const SharedString &message);

    LogLineData(const message::LogLine &m);
    message::LogLine toMessage() const;
};

class LogLine : private LogLineData {
  public:
    LogLine(const LogLineData &ll) : LogLineData(ll) {}
    int level() const { return level_; }
    std::chrono::system_clock::time_point time() const { return time_; }
    const std::string &message() const { return message_.decode(); };
};
class LogLineW : private LogLineData {
  public:
    LogLineW(const LogLineData &ll) : LogLineData(ll) {}
    int level() const { return level_; }
    std::chrono::system_clock::time_point time() const { return time_; }
    const std::wstring &message() const { return message_.decodeW(); };
};

/*!
 * \brief ログの送受信データを表すクラス
 *
 * * <del>fieldを継承しているがfield名は使用していない</del>
 * * ver2.4〜 他のデータ型と同じようにフィールド名を指定できるようになった
 *
 */
class WEBCFACE_DLL Log : protected Field {
  public:
    Log() = default;
    Log(const Field &base);
    /*!
     * \since ver2.4
     */
    Log(const Field &base, const SharedString &field)
        : Log(Field{base, field}) {}

    using Field::member;

    /*!
     * \brief Clientが保持するログの行数を設定する。
     * \since ver2.1
     * 
     * * この行数以上のログが送られてきたら古いログから順に削除され、get()で取得できなくなる。
     * * デフォルトは1000
     * * 負の値を設定すると無制限に保持する。
     * 
     */
    static void WEBCFACE_CALL keepLines(int n);

    /*!
     * \brief ログが追加されたときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback Log型の引数(thisが渡される)を1つ取る関数
     *
     */
    const Log &
    onChange(std::function<void WEBCFACE_CALL_FP(Log)> callback) const;
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     * \param callback 引数をとらない関数
     *
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                    std::nullptr_t> = nullptr>
    const Log &onChange(F callback) const {
        return onChange(
            [callback = std::move(callback)](const auto &) { callback(); });
    }
    /*!
     * \deprecated
     * ver1.11まではEventTarget::appendListener()でコールバックを追加できたが、
     * ver2.0からコールバックは1個のみになった。
     * 互換性のため残しているがonChange()と同じ
     *
     */
    template <typename T>
    [[deprecated]] void appendListener(T &&callback) const {
        onChange(std::forward<T>(callback));
    }

    /*!
     * \brief ログをリクエストする
     * \since ver1.7
     *
     */
    const Log &request() const;
    /*!
     * \brief ログを取得する
     *
     */
    std::optional<std::vector<LogLine>> tryGet() const;
    /*!
     * \brief ログを取得する (wstring)
     * \since ver2.0
     */
    std::optional<std::vector<LogLineW>> tryGetW() const;
    /*!
     * \brief ログを取得する
     *
     */
    std::vector<LogLine> get() const {
        return tryGet().value_or(std::vector<LogLine>{});
    }
    /*!
     * \brief ログを取得する (wstring)
     * \since ver2.0
     */
    std::vector<LogLineW> getW() const {
        return tryGetW().value_or(std::vector<LogLineW>{});
    }

    /*!
     * \brief このメンバーがログを1行以上出力していればtrue
     * \since ver2.1
     *
     * tryGet(), get().size() などとは違って、実際のログデータを受信しない。
     * リクエストも送信しない。
     *
     */
    bool exists() const;
    /*!
     * \brief 受信したログをクリアする
     * \since ver1.1.5
     *
     * リクエスト状態は解除しない
     */
    const Log &clear() const;

  private:
    const Log &append(LogLineData &&ll) const;

  public:
    /*!
     * \brief ログを1行追加
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    const Log &append(int level, std::string_view message) const {
        return append({level, std::chrono::system_clock::now(),
                       SharedString::encode(message)});
    }
    /*!
     * \brief ログを1行追加
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    const Log &append(int level, std::chrono::system_clock::time_point time,
                      std::string_view message) const {
        return append({level, time, SharedString::encode(message)});
    }
    /*!
     * \brief ログを1行追加 (wstring)
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    const Log &append(int level, std::wstring_view message) const {
        return append({level, std::chrono::system_clock::now(),
                       SharedString::encode(message)});
    }
    /*!
     * \brief ログを1行追加 (wstring)
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    const Log &append(int level, std::chrono::system_clock::time_point time,
                      std::wstring_view message) const {
        return append({level, time, SharedString::encode(message)});
    }

    /*!
     * \brief Logの参照先を比較
     * \since ver1.11
     */
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Log>,
                                                    std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
    template <typename T, typename std::enable_if_t<std::is_same_v<T, Log>,
                                                    std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return static_cast<Field>(*this) != static_cast<Field>(other);
    }
};
WEBCFACE_NS_END

#pragma once
#include <functional>
#include <optional>
#include <vector>
#include <chrono>
#include <ostream>
#include <string>
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

/*!
 * \brief ログレベルを表すenum
 * \since ver3.0
 *
 * * ver2.9までは namespace level 内の enum LogLevelEnum だった。
 * 後方互換性のために webcface::level としてエイリアスを貼っている
 * (他のクラスの名前規則と合っていないが)
 *
 */
enum class LogLevel : int {
    trace = 0,
    debug = 1,
    info = 2,
    warn = 3,
    warning = 3,
    error = 4,
    critical = 5,
};
using level = LogLevel;

/*!
 * \since ver3.0
 */
inline std::ostream &operator<<(std::ostream &os, LogLevel level) {
    switch (level) {
    case LogLevel::trace:
        return os << "trace";
    case LogLevel::debug:
        return os << "debug";
    case LogLevel::info:
        return os << "info";
    case LogLevel::warn:
        return os << "warn";
    case LogLevel::error:
        return os << "error";
    case LogLevel::critical:
        return os << "critical";
    default:
        return os << std::to_string(static_cast<int>(level));
    }
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
    LogLevel level() const { return static_cast<LogLevel>(level_); }
    std::chrono::system_clock::time_point time() const { return time_; }
    /*!
     * * ver3.0〜 StringViewに変更
     *
     */
    StringView message() const { return message_.decodeShare(); };
};
class LogLineW : private LogLineData {
  public:
    LogLineW(const LogLineData &ll) : LogLineData(ll) {}
    LogLevel level() const { return static_cast<LogLevel>(level_); }
    std::chrono::system_clock::time_point time() const { return time_; }
    /*!
     * * ver3.0〜 WStringViewに変更
     *
     */
    WStringView message() const { return message_.decodeShareW(); };
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
     * * sync()時にサーバーに送られる。コンソールへの出力などはされない
     * * ver3.0〜 String型に変更
     * * ver3.0〜 level引数を LogLevel とintの両方で受け付けるようにした
     *
     */
    const Log &append(LogLevel level, StringInitializer message) const {
        return append({static_cast<int>(level),
                       std::chrono::system_clock::now(),
                       static_cast<SharedString &>(message)});
    }
    const Log &append(int level, StringInitializer message) const {
        return append({level, std::chrono::system_clock::now(),
                       static_cast<SharedString &>(message)});
    }
    /*!
     * \brief ログを1行追加
     * \since ver2.0
     *
     * * sync()時にサーバーに送られる。コンソールへの出力などはされない
     * * ver3.0〜 String型に変更
     * * ver3.0〜 level引数を LogLevel とintの両方で受け付けるようにした
     *
     */
    const Log &append(LogLevel level,
                      std::chrono::system_clock::time_point time,
                      StringInitializer message) const {
        return append({static_cast<int>(level), time,
                       static_cast<SharedString &>(message)});
    }
    const Log &append(int level, std::chrono::system_clock::time_point time,
                      StringInitializer message) const {
        return append({level, time, static_cast<SharedString &>(message)});
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

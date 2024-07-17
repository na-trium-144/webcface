#pragma once
#include <functional>
#include <optional>
#include <vector>
#include "field.h"
#include "webcface/common/def.h"

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

class WEBCFACE_DLL LogLine : private LogLineData {
  public:
    LogLine(const LogLineData &ll) : LogLineData(ll) {}
    int level() const { return level_; }
    std::chrono::system_clock::time_point time() const { return time_; }
    const std::string &message() const { return message_.decode(); };
};
class WEBCFACE_DLL LogLineW : private LogLineData {
  public:
    LogLineW(const LogLineData &ll) : LogLineData(ll) {}
    int level() const { return level_; }
    std::chrono::system_clock::time_point time() const { return time_; }
    const std::wstring &message() const { return message_.decodeW(); };
};

/*!
 * \brief ログの送受信データを表すクラス
 *
 * fieldを継承しているがfield名は使用していない
 *
 */
class WEBCFACE_DLL Log : protected Field {
  public:
    Log() = default;
    Log(const Field &base);

    using Field::member;

    /*!
     * \brief ログが追加されたときに呼び出されるコールバックを設定
     * \since ver2.0
     */
    Log &onChange(std::function<void WEBCFACE_CALL_FP(Log)> callback);
    /*!
     * \brief 値が変化したときに呼び出されるコールバックを設定
     * \since ver2.0
     */
    template <typename F, typename std::enable_if_t<std::is_invocable_v<F>,
                                                  std::nullptr_t> = nullptr>
    Log &onChange(F callback) {
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
    [[deprecated]] void appendListener(T &&callback) {
        onChange(std::forward<T>(callback));
    }

    /*!
     * \brief ログをリクエストする
     * \since ver1.7
     *
     */
    void request() const;
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
     * \brief 受信したログをクリアする
     * \since ver1.1.5
     *
     * リクエスト状態は解除しない
     */
    Log &clear();

  private:
    Log &append(LogLineData &&ll);

  public:
    /*!
     * \brief ログを1行追加
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    Log &append(int level, std::string_view message) {
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
    Log &append(int level, std::chrono::system_clock::time_point time,
                std::string_view message) {
        return append({level, time, SharedString::encode(message)});
    }
    /*!
     * \brief ログを1行追加 (wstring)
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    Log &append(int level, std::wstring_view message) {
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
    Log &append(int level, std::chrono::system_clock::time_point time,
                std::wstring_view message) {
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

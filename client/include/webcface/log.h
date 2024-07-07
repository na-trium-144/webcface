#pragma once
#include <optional>
#include <vector>
#include "event_target.h"
#include "field.h"
#include <webcface/common/def.h>

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

template <typename CharT = char8_t>
class WEBCFACE_DLL_TEMPLATE LogLineData {
  protected:
    int level_ = 0;
    std::chrono::system_clock::time_point time_;
    SharedString message_;
    static_assert(std::same_as<CharT, char8_t> || std::same_as<CharT, char> ||
                  std::same_as<CharT, wchar_t>);

  public:
    LogLineData() = default;
    LogLineData(int level, std::chrono::system_clock::time_point time,
                const SharedString &message);
    LogLineData(const LogLineData &) = default;
    LogLineData &operator=(const LogLineData &) = default;
    LogLineData(LogLineData &&) noexcept = default;
    LogLineData &operator=(LogLineData &&) noexcept = default;
    ~LogLineData() = default;

    template <typename OtherCharT>
        requires(!std::same_as<CharT, OtherCharT>)
    operator LogLineData<OtherCharT>() const {
        return LogLineData<OtherCharT>(level_, time_, message_);
    }

    LogLineData(const message::LogLine &m);
    message::LogLine toMessage() const;

    int level() const { return level_; }
    std::chrono::system_clock::time_point time() const { return time_; }
    const std::basic_string<CharT> message() const {
        if constexpr (std::same_as<CharT, char8_t>) {
            return message_.u8String();
        } else if constexpr (std::same_as<CharT, char>) {
            return message_.decode();
        } else /* if constexpr (std::same_as<CharT, wchar_t>) */ {
            return message_.decodeW();
        }
    };
};

#ifdef _WIN32
extern template class WEBCFACE_DLL_INSTANCE_DECL LogLineData<char8_t>;
extern template class WEBCFACE_DLL_INSTANCE_DECL LogLineData<char>;
extern template class WEBCFACE_DLL_INSTANCE_DECL LogLineData<wchar_t>;
#endif

using LogLine = LogLineData<char>;
using LogLineW = LogLineData<wchar_t>;

/*!
 * \brief ログの送受信データを表すクラス
 *
 * fieldを継承しているがfield名は使用していない
 *
 */
class WEBCFACE_DLL Log : protected Field, public EventTarget<Log> {
    void onAppend() const override final;

  public:
    Log() = default;
    Log(const Field &base);

    using Field::member;

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
    Log &append(LogLineData<> &&ll);

  public:
    /*!
     * \brief ログを1行追加
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    Log &append(int level, std::string_view message) {
        return append(
            {level, std::chrono::system_clock::now(), SharedString(message)});
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
        return append({level, time, SharedString(message)});
    }
    /*!
     * \brief ログを1行追加 (wstring)
     * \since ver2.0
     *
     * sync()時にサーバーに送られる。コンソールへの出力などはされない
     *
     */
    Log &append(int level, std::wstring_view message) {
        return append(
            {level, std::chrono::system_clock::now(), SharedString(message)});
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
        return append({level, time, SharedString(message)});
    }

    /*!
     * \brief Logの参照先を比較
     * \since ver1.11
     */
    template <typename T>
        requires std::same_as<T, Log>
    bool operator==(const T &other) const {
        return static_cast<Field>(*this) == static_cast<Field>(other);
    }
};
WEBCFACE_NS_END

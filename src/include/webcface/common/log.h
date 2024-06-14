#pragma once
#include <string>
#include <chrono>
#include <concepts>
#include <webcface/common/def.h>
#include <webcface/encoding.h>

WEBCFACE_NS_BEGIN
inline namespace Common {

template <typename CharT = char8_t>
class LogLineData {
  protected:
    int level_ = 0;
    std::chrono::system_clock::time_point time_;
    SharedString message_;
    static_assert(std::same_as<CharT, char8_t> || std::same_as<CharT, char> ||
                  std::same_as<CharT, wchar_t>);

  public:
    LogLineData() = default;
    LogLineData(int level, std::chrono::system_clock::time_point time,
                const SharedString &message)
        : level_(level), time_(time), message_(message) {}

    template <typename OtherCharT>
        requires(!std::same_as<CharT, OtherCharT>)
    operator LogLineData<OtherCharT>() const {
        return LogLineData<OtherCharT>(level_, time_, message_);
    }

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
using LogLine = LogLineData<char>;
using LogLineW = LogLineData<wchar_t>;

} // namespace Common
WEBCFACE_NS_END

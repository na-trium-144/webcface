#pragma once
#include <string>
#include <chrono>
#include <type_traits>
#include <webcface/common/def.h>
#include <webcface/encoding.h>

WEBCFACE_NS_BEGIN
inline namespace Common {
template <typename CharT = char8_t>
struct LogLineData {
    int level = 0;
    std::chrono::system_clock::time_point time;
    std::basic_string<CharT> message;
    LogLineData() = default;
    LogLineData(const LogLineData<char8_t> &data)
        : LogLineData(data.level, data.time, data.message) {}
    LogLineData(int level, std::chrono::system_clock::time_point time,
                std::u8string_view message)
        : level(level), time(time) {
        if constexpr (std::is_same_v<CharT, char>) {
            this->message = Encoding::decode(message);
        } else if constexpr (std::is_same_v<CharT, wchar_t>) {
            this->message = Encoding::decodeW(message);
        } else {
            this->message = message;
        }
    }
};
using LogLine = LogLineData<char>;
using LogLineW = LogLineData<wchar_t>;

} // namespace Common
WEBCFACE_NS_END

#pragma once
#include <string>
#include <chrono>
#include "def.h"

WEBCFACE_NS_BEGIN
inline namespace Common {
struct LogLine {
    int level = 0;
    std::chrono::system_clock::time_point time;
    std::u8string message;
    LogLine() = default;
    LogLine(int level, std::chrono::system_clock::time_point time,
            std::u8string_view message)
        : level(level), time(time), message(message) {}
};
} // namespace Common
WEBCFACE_NS_END

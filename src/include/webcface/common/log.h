#pragma once
#include <string>
#include <chrono>

namespace webcface {
inline namespace Common {
struct LogLine {
    int level = 0;
    std::chrono::system_clock::time_point time;
    std::string message;
    LogLine() = default;
    LogLine(int level, std::chrono::system_clock::time_point time,
            const std::string &message)
        : level(level), time(time), message(message) {}
};
} // namespace Common
} // namespace webcface
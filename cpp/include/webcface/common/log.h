#pragma once
#include <string>
#include <chrono>

namespace WebCFace {
inline namespace Common {
struct LogLine {
    int level;
    std::chrono::system_clock::time_point time;
    std::string message;
};
} // namespace Common
} // namespace WebCFace
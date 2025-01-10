#pragma once
#include <deque>
#include <vector>
#include "webcface/log_line.h"

WEBCFACE_NS_BEGIN
namespace internal {

struct LogHistory {
    std::deque<LogLineData> data;
    std::size_t sent_lines = 0;

    LogHistory() = default;
    explicit LogHistory(const std::deque<LogLineData> &data) : data(data) {}

    std::vector<LogLineData> getDiff() {
        auto begin = data.cbegin() + static_cast<int>(sent_lines);
        auto end = data.cend();
        sent_lines = data.size();
        return std::vector<LogLineData>(begin, end);
    }
    std::vector<LogLineData> getAll() {
        sent_lines = data.size();
        return std::vector<LogLineData>(data.cbegin(), data.cend());
    }
};

}
WEBCFACE_NS_END

#pragma once
#include <chrono>
#include "webcface/common/encoding.h"
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

WEBCFACE_NS_END

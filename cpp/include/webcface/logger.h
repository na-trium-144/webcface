#pragma once
#include <streambuf>
#include <memory>
#include <string>

namespace WebCFace {
class ClientData;
struct LogLine {
    int level;
    std::string message;
};
class LoggerBuf : public std::streambuf {
    static constexpr int buf_size = 1024;
    char buf[buf_size];
    // bufからあふれた分を入れる
    std::string overflow_buf;

    std::shared_ptr<ClientData> data;
    int level;

    int sync() override;
    int overflow(int c) override;

  public:
    LoggerBuf(const std::shared_ptr<ClientData> &data, int level);
    LoggerBuf(const LoggerBuf &) = delete;
    LoggerBuf &operator=(const LoggerBuf &) = delete;
};

} // namespace WebCFace
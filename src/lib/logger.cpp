#include "logger.hpp"
#include <webcface/server.hpp>
#include <cstddef>
#include <thread>
namespace WebCFace
{
inline namespace Logger
{
void initStdLogger()
{
    std_logger.set();
}
void appendLogLine(int level, std::string text)
{
    std_logger.appendLogLine(level, text);
}

void StdLogger::appendLogLine(int level, std::string text)
{
    auto appendLog = [level, text, this] {
        log_buffer.push_back({getTime(), level, text});
        while (log_buffer.size() > 100) {
            // 1周期に1000行書き込まれると消えるので良くない
            log_buffer.pop_front();
            if (log_pos > 0) {
                --log_pos;
            }
        }
    };
    if (internal_mutex.try_lock()) {
        appendLog();
        internal_mutex.unlock();
    } else {
        // mutexが取れなければ後回しにする
        std::thread([=] {
            std::lock_guard lock(internal_mutex);
            appendLog();
        }).detach();
    }

    // t.detach();
}

teebuf::teebuf()
{
    setp(buf, buf + sizeof(buf));
}
int teebuf::sync()
{
    static std::string line = "";
    std::string new_line = buf;
    line += new_line;
    std::memset(buf, 0, sizeof(buf));
    setp(buf, buf + sizeof(buf));
    if (sb1) {
        sb1->sputn(new_line.c_str(), new_line.size());
    }
    while (true) {
        auto i = line.find('\n');
        if (i == std::string::npos) {
            break;
        }
        std_logger.appendLogLine(0, line.substr(0, i));
        line = line.substr(i + 1);
    }
    if (sb1) {
        int const r1 = sb1->pubsync();
        return r1;
    }
    return 0;
}
}  // namespace Logger
}  // namespace WebCFace

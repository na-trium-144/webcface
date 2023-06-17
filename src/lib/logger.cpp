#include "logger.hpp"
#include <cstddef>
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
    log_buffer.push_back({getTime(), level, text});
}

int teebuf::overflow(int c)
{
    if (c == EOF) {
        return !EOF;
    } else {
        int const r1 = sb1->sputc(c);
        if (c == '\n') {
            std_logger.appendLogLine(0, tmp_s);
            tmp_s = "";
        } else {
            tmp_s += static_cast<char>(c);
        }
        return r1;
    }
}
}  // namespace Logger
}  // namespace WebCFace

#include <webcface/logger.h>
#include <webcface/client_data.h>
#include <webcface/client.h>

namespace WebCFace {
LoggerBuf::LoggerBuf(const std::shared_ptr<ClientData> &data, int level)
    : std::streambuf(), data(data), level(level) {
    this->setp(buf, buf + sizeof(buf));
}
int LoggerBuf::overflow(int c) {
    overflow_buf += std::string(buf, this->pptr() - this->pbase());
    this->setp(buf, buf + sizeof(buf));
    this->sputc(c);
    return 0;
}
int LoggerBuf::sync() {
    overflow_buf += std::string(buf, this->pptr() - this->pbase());
    while (true) {
        std::size_t n = overflow_buf.find_first_of('\n');
        if (n == std::string::npos) {
            break;
        }
        std::string message = overflow_buf.substr(0, n);
        data->log_send_queue.push({level, message});
        data->log_display_queue.push({level, message});
        overflow_buf = overflow_buf.substr(n + 1);
    }
    this->setp(buf, buf + sizeof(buf));
    return 0;
}

} // namespace WebCFace
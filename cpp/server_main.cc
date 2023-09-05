#include "../server/websock.h"
#include <spdlog/sinks/stdout_color_sinks.h>

int main() {
    auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

    // Set HTTP listener address and port
    // todo: 引数で変えられるようにする
    WebCFace::Server::serverRun(7530, stderr_sink, spdlog::level::trace);
    return 0;
}

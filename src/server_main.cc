#include "../server/websock.h"
#include <webcface/common/def.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <CLI/CLI.hpp>

int main(int argc, char **argv) {
    CLI::App app{"WebCFace Server " WEBCFACE_VERSION};
    app.allow_windows_style_options();

    int port = WEBCFACE_DEFAULT_PORT;
    app.add_option("-p,--port", port,
                   "Server port (default: " WEBCFACE_DEFAULT_PORT_S ")");

    CLI11_PARSE(app, argc, argv);

    auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

    // Set HTTP listener address and port
    // todo: 引数で変えられるようにする
    WebCFace::Server::serverRun(port, stderr_sink, spdlog::level::trace);
}

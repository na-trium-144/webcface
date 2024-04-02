#include "../server/websock.h"
#include "../server/store.h"
#include <webcface/common/def.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <CLI/CLI.hpp>

int main(int argc, char **argv) {
    std::string server_ver = "WebCFace Server " WEBCFACE_VERSION ", OpenCV ";
    if constexpr (WEBCFACE_USE_OPENCV) {
        server_ver += "Enabled";
    } else {
        server_ver += "Disabled";
    }
    CLI::App app{server_ver};
    app.allow_windows_style_options();

    int port = WEBCFACE_DEFAULT_PORT;
    int verbosity = 0;
    int keep_log = 1000;
    app.add_option("-p,--port", port,
                   "Server port (default: " WEBCFACE_DEFAULT_PORT_S ")");
    app.add_flag("-v,--verbose", verbosity,
                 "Show all received messages (-vv to show sent messages too)");
    app.add_option("-l,--keep-log", keep_log,
                   ("Number of lines of received log to keep (default: " +
                    std::to_string(keep_log) +
                    ") \n"
                    "(keep all log by setting -1)")
                       .c_str());

    CLI11_PARSE(app, argc, argv);

    auto stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
    if (verbosity >= 2) {
        stderr_sink->set_level(spdlog::level::trace);
    } else if (verbosity == 1) {
        stderr_sink->set_level(spdlog::level::debug);
    } else {
        stderr_sink->set_level(spdlog::level::info);
    }

    webcface::Server::store.keep_log = keep_log;
    webcface::Server::serverRun(port, stderr_sink, spdlog::level::trace);
}

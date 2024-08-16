#include "./config.h"
#include <webcface/server.h>
#include <CLI/CLI.hpp>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define DEFAULT_TOML "webcface.toml"

int main(int argc, char **argv) {
    spdlog::sink_ptr sink;
    std::shared_ptr<spdlog::logger> logger;
    try {
        sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
        logger = std::make_shared<spdlog::logger>("webcface_server", sink);

        CLI::App app{"WebCFace Server " WEBCFACE_VERSION};
        app.allow_windows_style_options();

        std::uint16_t port = WEBCFACE_DEFAULT_PORT;
        int verbosity = 0;
        int keep_log = 1000;

        app.add_option("-p,--port", port,
                       "Server port (default: " WEBCFACE_DEFAULT_PORT_S ")");
        app.add_flag(
            "-v,--verbose", verbosity,
            "Show all received messages (-vv to show sent messages too)");
        app.add_option("-l,--keep-log", keep_log,
                       ("Number of lines of received log to keep (default: " +
                        std::to_string(keep_log) +
                        ") \n"
                        "(keep all log by setting -1)")
                           .c_str());

        bool use_stdin = false;
        app.add_flag("-s,--stdin", use_stdin,
                     "Read config from stdin instead of file");
        std::string toml_path = DEFAULT_TOML;
        app.add_option("config_path", toml_path,
                       "Path of config file (default: " DEFAULT_TOML ")");

        CLI11_PARSE(app, argc, argv);

        int level;
        if (verbosity >= 2) {
            level = 0;
        } else if (verbosity == 1) {
            level = 1;
        } else {
            level = 2;
        }

        auto toml_config = parseToml(toml_path, use_stdin);
        ServerConfig config = parseConfig(toml_config);
        webcface::server::Server(port, level, config.commands, keep_log).join();

    } catch (const std::exception &e) {
        if (sink && logger) {
            logger->critical("Unhandled exception: {}", e.what());
        } else {
            std::cerr << "Unhandled exception: " << e.what() << std::endl;
        }
    }
}

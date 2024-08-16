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

        ServerConfig config = parseConfig(toml_path, use_stdin);

        webcface::server::Server(port, level, keep_log).join();


        initHandler();

        while (!shouldStop()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            launcherLoop(wcli, commands);
        }

#ifndef _WIN32
        int sig_send = 0;
        do {
            if (shouldStop()) {
                spdlog::warn("Signal {} received.", sig_received);
                if (sig_send == 0) {
                    if (sig_received == SIGINT) {
                        sig_send = SIGINT;
                    } else {
                        sig_send = SIGTERM;
                    }
                } else if (sig_send != SIGTERM) {
                    // 再度シグナルが送られたら、sigtermにしてもう1回送る
                    sig_send = SIGTERM;
                    spdlog::warn("Escalating to {}.", sig_send);
                } else {
                    // termでも止まらなかったらkill?
                    sig_send = SIGKILL;
                    spdlog::warn("Escalating to {}.", sig_send);
                }
                for (auto &cmd : commands) {
                    if (cmd->start_p->is_running()) {
                        cmd->start_p->kill(sig_send);
                    }
                }
                sig_received = 0;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } while (
            std::any_of(commands.begin(), commands.end(), [](const auto &cmd) {
                return cmd->start_p->is_running();
            }));
        return 1;
#endif


    } catch (const std::exception &e) {
        if (sink && logger) {
            logger->critical("Unhandled exception: {}", e.what());
        } else {
            std::cerr << "Unhandled exception: " << e.what() << std::endl;
        }
    }
}

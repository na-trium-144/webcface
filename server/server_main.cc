#include <webcface/server.h>
#include <webcface/common/def.h>
#include <CLI/CLI.hpp>

int main(int argc, char **argv) {
    CLI::App app{"WebCFace Server " WEBCFACE_VERSION};
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

    int level;
    if (verbosity >= 2) {
        level = 0;
    } else if (verbosity == 1) {
        level = 1;
    } else {
        level = 2;
    }

    webcface::server::Server(port, level, keep_log).join();
}

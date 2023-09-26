#include "../server/websock.h"
#include "../include/webcface/common/def.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <tclap/CmdLine.h>

int main(int argc, char **argv) {
    try {
        TCLAP::CmdLine cmd("WebCFace Server", ' ', WEBCFACE_VERSION);
        TCLAP::ValueArg<int> portArg(
            "p", "port",
            "Server port (default: " + std::to_string(WEBCFACE_DEFAULT_PORT) +
                ")",
            false, WEBCFACE_DEFAULT_PORT, "number");
        cmd.add(portArg);

        cmd.parse(argc, argv);

        auto stderr_sink =
            std::make_shared<spdlog::sinks::stderr_color_sink_mt>();

        // Set HTTP listener address and port
        // todo: 引数で変えられるようにする
        WebCFace::Server::serverRun(portArg.getValue(), stderr_sink,
                                    spdlog::level::trace);
        return 0;
    } catch (TCLAP::ArgException &e) {
        std::cerr << "error: " << e.error() << " for arg " << e.argId()
                  << std::endl;
    }
}

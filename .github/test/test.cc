#include <webcface/client.h>
#include <webcface/value.h>
#include <webcface/logger.h>
#include <webcface/server.h>
#include "spdlog/sinks/stdout_sinks.h"

int main() {
    webcface::Client wcli{};
    wcli.value("test") = 0;
    wcli.logger()->info("this is info");
    wcli.onMemberEntry().callbackList().append([](webcface::Member m) {});

    auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    webcface::Server::Server s(7530, sink, spdlog::level::info);
}

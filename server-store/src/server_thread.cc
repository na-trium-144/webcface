#include "webcface/server/store.h"
#include "webcface/server/member_data.h"
#include "webcface/server/dir.h"
#include "webcface/server/ip.h"
#include "webcface/server/server.h"
#include "webcface/message/message.h"
#include "webcface/internal/unix_path.h"
#include <memory>
#include <thread>
#include "webcface/server/server_ws.h"
#include <spdlog/sinks/stdout_color_sinks.h>

WEBCFACE_NS_BEGIN
namespace server {

constexpr std::chrono::milliseconds LAUNCHER_INTERVAL{50};

void Server::pingThreadMain() {
    std::unique_lock lock(server_mtx);
    std::chrono::steady_clock::time_point last_ping =
        std::chrono::steady_clock::now();
    while (!server_stop.load()) {
        // 50ms or server_stop_condで起こされるまで待機
        server_ping_wait.wait_for(lock, LAUNCHER_INTERVAL);
        if (server_stop.load()) {
            return;
        }
        updateCommandStatus();
        if (std::chrono::steady_clock::now() - last_ping >=
            MemberData::ping_interval) {
            ping();
            last_ping += MemberData::ping_interval;
        }
    }
    shutdownCommands();
}

void Server::ping() {
    auto new_ping_status =
        std::make_shared<std::unordered_map<unsigned int, int>>();
    store->forEach([&](auto cd) {
        if (cd->last_ping_duration) {
            new_ping_status->emplace(
                cd->member_id,
                static_cast<int>(cd->last_ping_duration->count()));
        }
    });
    store->ping_status = new_ping_status;
    auto msg = message::packSingle(message::PingStatus{{}, store->ping_status});
    store->forEach([&](auto cd) {
        cd->logger->trace("ping");
        cd->sendPing();
        if (cd->ping_status_req) {
            cd->send(msg);
            cd->logger->trace("send ping_status");
        }
    });
}
void Server::updateCommandStatus() {
    for (const auto &c : store->commands) {
        c->checkStatusChanged([&](bool running, int status) {
            store->forEach([&](auto cd) {
                if (cd->command_status_req.count(c->id())) {
                    cd->logger->trace(
                        "send command_status {}: running = {}, status = {}",
                        c->name(), running, status);
                    cd->pack(
                        message::CommandStatus{{}, c->id(), running, status});
                }
            });
        });
        c->checkLogs([&](const std::deque<LogLineData> &logs,
                         std::size_t prev_log_lines) {
            store->forEach([&](auto cd) {
                if (cd->command_log_req.count(c->id())) {
                    cd->pack(message::CommandLog{
                        c->id(),
                        logs.begin() + static_cast<int>(prev_log_lines),
                        logs.end()});
                }
            });
        });
    }
    store->forEach([&](auto cd) { cd->send(); });
}

void Server::shutdownCommands() {
#ifndef _WIN32
    std::chrono::steady_clock::time_point shutdown_start =
        std::chrono::steady_clock::now();
    auto check_shutdown = [&](std::chrono::seconds timeout){
        while (std::chrono::steady_clock::now() - shutdown_start < timeout) {
        if (std::all_of(store->commands.begin(), store->commands.end(),
                        [](const auto &c) { return c->shutdownOk(); })) {
            return;
        }
        std::this_thread::sleep_for(LAUNCHER_INTERVAL);
    }
    };
    for (const auto &c : store->commands) {
        c->kill(SIGINT);
    }
    check_shutdown(std::chrono::seconds(5));
    this->logger->warn("Timeout, Escalating to SIGTERM");
    for (const auto &c : store->commands) {
        c->kill(SIGTERM);
    }
    check_shutdown(std::chrono::seconds(10));
    this->logger->warn("Timeout, Escalating to SIGKILL");
    for (const auto &c : store->commands) {
        c->kill(SIGKILL);
    }
#endif
}

} // namespace server
WEBCFACE_NS_END

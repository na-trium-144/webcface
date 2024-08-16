#pragma once
#include <csignal>
#include <cstdlib>
#include <spdlog/spdlog.h>

inline volatile std::sig_atomic_t sig_received = 0;
inline bool shouldStop() { return sig_received != 0; }
inline void handler(int sig) { sig_received = sig; }

inline void initHandler() {
    struct sigaction act;
    std::memset(&act, 0, sizeof(act));
    act.sa_handler = handler;
    const int signals[] = {SIGTERM, SIGINT, SIGQUIT, SIGHUP, SIGPIPE};
    for (auto s : signals) {
        if (sigaction(s, &act, nullptr) < 0) {
            spdlog::error("Error in sigaction(sig={}): {}", s, errno);
        }
    }
}

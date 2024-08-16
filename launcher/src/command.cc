#include "webcface/launcher/command.h"
#include <process.hpp>
#include <spdlog/sinks/stdout_color_sinks.h>

#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
#include <windows.h>
#endif

WEBCFACE_NS_BEGIN
namespace launcher {

Process::Process(const std::string &name, const std::string &exec,
                 const std::string &workdir, CaptureMode capture_stdout,
                 bool stdout_is_utf8,
                 const std::unordered_map<std::string, std::string> &env,
                 spdlog::sink_ptr sink)
    : std::enable_shared_from_this<Process>(), exec(exec), workdir(workdir),
      capture_stdout(capture_stdout), stdout_is_utf8(stdout_is_utf8), env(env),
      name(name), logger(std::make_shared<spdlog::logger>(name, sink)) {
    // logger->set_pattern("[%n] %v");
}

void Process::start() {
    std::lock_guard lock(m);
    auto read_log = [proc = shared_from_this()](const char *bytes,
                                                std::size_t n) {
        std::lock_guard lock(proc->m);
        std::string_view log_data(bytes, n);
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
        std::string result_utf8;
        if (!proc->stdout_is_utf8) {
            // acp → utf-8
            auto length = MultiByteToWideChar(CP_ACP, 0, bytes,
                                              static_cast<int>(n), nullptr, 0);
            std::wstring result_utf16(length, '\0');
            MultiByteToWideChar(CP_ACP, 0, bytes, static_cast<int>(n),
                                result_utf16.data(),
                                static_cast<int>(result_utf16.length()));
            auto length_utf8 =
                WideCharToMultiByte(CP_UTF8, 0, result_utf16.data(),
                                    static_cast<int>(result_utf16.size()),
                                    nullptr, 0, nullptr, nullptr);
            result_utf8.assign(length_utf8, '\0');
            WideCharToMultiByte(
                CP_UTF8, 0, result_utf16.data(),
                static_cast<int>(result_utf16.size()), result_utf8.data(),
                static_cast<int>(result_utf8.size()), nullptr, nullptr);
            log_data = result_utf8;
        }
#endif
        std::size_t i;
        while ((i = log_data.find_first_of('\n')) != std::string_view::npos) {
            proc->logs.emplace_back(
                level::info, std::chrono::system_clock::now(),
                SharedString::fromU8String(log_data.substr(0, i)));
            proc->logger->info(log_data.substr(0, i));
            log_data = log_data.substr(i + 1);
        }
        if (!log_data.empty()) {
            proc->logs.emplace_back(level::info,
                                    std::chrono::system_clock::now(),
                                    SharedString::fromU8String(log_data));
            proc->logger->info(log_data);
        }
    };
    int exit_status;
    if (p && !p->try_get_exit_status(exit_status)) {
        this->logger->warn("Command '{}' is already started.", this->name);
        // throw std::runtime_error("already started");
    } else {
        this->logger->info("Starting command '{}'.", this->name);
        this->logs.clear();
        if (this->capture_stdout != CaptureMode::never) {
            p = std::make_shared<TinyProcessLib::Process>(
                this->exec, this->workdir, this->env, read_log, read_log);
        } else {
            p = std::make_shared<TinyProcessLib::Process>(
                this->exec, this->workdir, this->env);
        }
    }
}
void Process::kill([[maybe_unused]] int sig) {
    std::lock_guard lock(m);
    int exit_status;
    if (p && !p->try_get_exit_status(exit_status)) {
#ifdef _WIN32
        this->logger->info("Stopping command '{}'.", this->name);
        p->kill(false);
#else
        this->logger->info("Sending signal {} to command '{}'.", sig,
                           this->name);
        p->signal(sig);
#endif
    } else {
        this->logger->warn("Command '{}' is already stopped.", this->name);
        // throw std::runtime_error("already stopped");
    }
}

std::pair<bool, int> Process::isRunning() {
    std::lock_guard lock(m);
    int exit_status = -1;
    if (p && !p->try_get_exit_status(exit_status)) {
        return std::make_pair(true, 0);
    } else {
        return std::make_pair(false, exit_status);
    }
}

void Command::start() {
    std::lock_guard lock(m);
    this->start_p->start();
}
void Command::stop() {
    std::lock_guard lock(m);
    switch (this->stop_p.index()) {
    case 1:
        std::get<1>(this->stop_p)->start();
        break;
    case 2:
        this->start_p->kill(std::get<2>(this->stop_p));
        break;
    default:
        // throw std::runtime_error("stop signal disabled");
        this->start_p->logger->warn("stop signal is disabled");
        break;
    }
}

void Command::checkStatusChanged(const std::function<void(bool, int)> &func) {
    std::lock_guard lock(m);
    auto [running, status] = start_p->isRunning();
    if (prev_running != running) {
        prev_running = running;
        func(running, status);
    }
}
void Command::checkLogs(
    const std::function<void(std::deque<LogLineData> &, std::size_t)> &func) {
    std::lock_guard lock(m);
    std::lock_guard proc_lock(start_p->m);
    if (start_p->logs.size() > prev_log_lines) {
        func(start_p->logs, prev_log_lines);
    }
    prev_log_lines = start_p->logs.size();
}
std::deque<LogLineData> Command::getAllLogs() {
    std::lock_guard lock(m);
    std::lock_guard proc_lock(start_p->m);
    return start_p->logs;
}

} // namespace launcher
WEBCFACE_NS_END

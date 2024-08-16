#include "webcface/launcher/command.h"
#include <process.hpp>

WEBCFACE_NS_BEGIN
namespace launcher {

Process::Process(const std::string &name, const std::string &exec,
                 const std::string &workdir, CaptureMode capture_stdout,
                 bool stdout_is_utf8,
                 const std::unordered_map<std::string, std::string> &env)
    : std::enable_shared_from_this<Process>(), name(name), exec(exec),
      workdir(workdir), capture_stdout(capture_stdout),
      stdout_is_utf8(stdout_is_utf8), env(env),
      logger(spdlog::stdout_color_mt(name)) {
    logger->set_pattern("[%n] %v");
}

void Process::start() {
    auto read_log = [cmd = shared_from_this()](const char *bytes,
                                               std::size_t n) {
#ifdef _WIN32
        if (!cmd->stdout_is_utf8) {
            cmd->logs += acpToUTF8(bytes, static_cast<int>(n));
        } else {
            cmd->logs.append(bytes, n);
        }
#else
        cmd->logs.append(bytes, n);
#endif
        cmd->logger->info(std::string(bytes, n));
    };
    if (is_running()) {
        spdlog::warn("Command '{}' is already started.", this->name);
        throw std::runtime_error("already started");
    } else {
        spdlog::info("Starting command '{}'.", this->name);
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
    if (is_running()) {
#ifdef _WIN32
        spdlog::info("Stopping command '{}'.", this->name);
        p->kill(false);
#else
        spdlog::info("Sending signal {} to command '{}'.", sig, this->name);
        p->signal(sig);
#endif
    } else {
        spdlog::warn("Command '{}' is already stopped.", this->name);
        throw std::runtime_error("already stopped");
    }
}

bool Process::is_running() { return p && !p->try_get_exit_status(exit_status); }

void Command::initFunc(WebCFace::Client &wcli) {
    start_f =
        wcli.func(start_p->name + "/start").set([cmd = shared_from_this()] {
            cmd->start_p->start();
        });
    stop_f = wcli.func(start_p->name + "/stop").set([cmd = shared_from_this()] {
        switch (cmd->stop_p.index()) {
        case 1:
            std::get<1>(cmd->stop_p)->start();
            break;
        case 2:
            cmd->start_p->kill(std::get<2>(cmd->stop_p));
            break;
        default:
            throw std::runtime_error("stop signal disabled");
        }
    });
}

} // namespace launcher
WEBCFACE_NS_END

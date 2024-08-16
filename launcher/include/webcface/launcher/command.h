#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include <spdlog/spdlog.h>
#include <string>
#include <memory>
#include <unordered_map>

namespace TinyProcessLib {
class Process;
}

WEBCFACE_NS_BEGIN
namespace launcher {

enum class CaptureMode {
    never,
    onerror,
    always,
};
struct Process : std::enable_shared_from_this<Process> {
    std::string name;
    std::string exec;
    std::string workdir;
    CaptureMode capture_stdout;
    bool stdout_is_utf8;
    std::unordered_map<std::string, std::string> env;
    int exit_status = 0;
    std::shared_ptr<TinyProcessLib::Process> p;
    std::shared_ptr<spdlog::logger> logger;
    std::string logs;

    Process(const Process &) = delete;
    Process &operator=(const Process &) = delete;
    Process(const std::string &name, const std::string &exec,
            const std::string &workdir, CaptureMode capture_stdout,
            bool stdout_is_utf8,
            const std::unordered_map<std::string, std::string> &env);

    void start();
    void kill(int sig);
    bool is_running();
};

// ProcessにStart/Stopボタンの実装を追加したもの
struct Command : std::enable_shared_from_this<Command> {
    webcface::Func start_f, stop_f;
    std::shared_ptr<Process> start_p;
    using StopOption =
        std::variant<std::nullopt_t, std::shared_ptr<Process>, int>;
    StopOption stop_p;

    Command(const Command &) = delete;
    Command &operator=(const Command &) = delete;
    Command(const std::shared_ptr<Process> &start_p, const StopOption &stop_p)
        : start_p(start_p), stop_p(stop_p) {}

    // shared_from_thisを使うためコンストラクタと別
    void initFunc(WebCFace::Client &wcli);
};

} // namespace launcher
WEBCFACE_NS_END

#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "webcface/log.h"
#include <deque>
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
class Process : std::enable_shared_from_this<Process> {
    std::string exec;
    std::string workdir;
    CaptureMode capture_stdout;
    bool stdout_is_utf8;
    std::unordered_map<std::string, std::string> env;
    std::shared_ptr<TinyProcessLib::Process> p;

  public:
    std::mutex m;
    SharedString name;
    std::deque<LogLineData> logs;
    std::shared_ptr<spdlog::logger> logger;

    Process(const Process &) = delete;
    Process &operator=(const Process &) = delete;
    Process(const SharedString &name, const std::string &exec,
            const std::string &workdir, CaptureMode capture_stdout,
            bool stdout_is_utf8,
            const std::unordered_map<std::string, std::string> &env,
            spdlog::sink_ptr sink);

    void start();
    void kill(int sig);
    /*!
     * \brief 実行中かどうかと、ステータスコード
     *
     */
    std::pair<bool, int> isRunning();
};

using StopOption = std::variant<std::nullopt_t, std::shared_ptr<Process>, int>;
/*!
 * ProcessにStart/Stopボタンの実装を追加したもの
 *
 */
class Command : std::enable_shared_from_this<Command> {
    std::size_t id_;
    std::shared_ptr<Process> start_p;
    StopOption stop_p;

    std::mutex m;
    bool prev_running = false;
    std::size_t prev_log_lines = 0;

  public:
    Command(const Command &) = delete;
    Command &operator=(const Command &) = delete;
    Command(std::size_t id, const std::shared_ptr<Process> &start_p,
            const StopOption &stop_p)
        : id_(id), start_p(start_p), stop_p(stop_p) {}

    SharedString name() { return start_p->name; }
    std::size_t id() { return id_; }
    void start();
    void stop();

    /*!
     * start_pだけでなくstop_pも動いていればkill
     */
    void kill(int sig);
    /*!
     * start_pだけでなくstop_pも動いてないか確認する
     */
    bool shutdownOk();

    std::pair<bool, int> isRunning();
    void getAllLogs(
        const std::function<void(const std::deque<LogLineData> &)> &func);

    /*!
     * \brief
     * 実行中かどうかのステータスが前回呼び出し時から変化したらコールバックに渡す
     *
     */
    void checkStatusChanged(const std::function<void(bool, int)> &func);
    /*!
     * \brief
     * プロセスのログが前回呼び出し時から増えていたらコールバックに渡す
     *
     */
    void checkLogs(const std::function<void(const std::deque<LogLineData> &,
                                            std::size_t)> &func);
};

} // namespace launcher
WEBCFACE_NS_END

#include "webcface/launcher/launcher.h"
#include "webcface/launcher/command.h"
#include <toml++/toml.hpp>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <spdlog/spdlog.h>
#include <sstream>

WEBCFACE_NS_BEGIN
namespace launcher {

std::shared_ptr<Process> parseTomlProcess(toml::node &config_node,
                                          const std::string &default_name) {
    auto t_name = config_node[toml::path("name")];
    std::string name = default_name;
    if (!t_name.is_string() && default_name.empty()) {
        spdlog::error("Error reading 'name'");
        std::exit(1);
    }
    if (t_name.is_string()) {
        name = **t_name.as_string();
    }

    auto t_exec = config_node[toml::path("exec")];
    if (!t_exec.is_string()) {
        spdlog::error("Error reading 'exec'");
        std::exit(1);
    }
    auto exec = **t_exec.as_string();
    spdlog::info("Command '{}': {}", name, exec);

    auto t_wd = config_node[toml::path("workdir")];
    std::string workdir = ".";
    if (t_wd) {
        if (!t_wd.is_string()) {
            spdlog::error("Error reading 'workdir'");
            std::exit(1);
        }
        workdir = **t_wd.as_string();
        spdlog::info(" workdir: {}", workdir);
    }

    auto t_cap = config_node[toml::path("stdout_capture")];
    CaptureMode capture = CaptureMode::onerror;
    if (t_cap) {
        if (!t_cap.is_string()) {
            spdlog::error("Error reading 'stdout_capture'");
            std::exit(1);
        }
        auto capture_stdout = **t_cap.as_string();
        if (capture_stdout == "never") {
            capture = CaptureMode::never;
        } else if (capture_stdout == "onerror") {
            capture = CaptureMode::onerror;
        } else if (capture_stdout == "always") {
            capture = CaptureMode::always;
        } else {
            spdlog::error(
                "'stdout_capture' must be 'never', 'onerror' or 'always'.");
            std::exit(1);
        }
        spdlog::info(" stdout_capture: {}", capture_stdout);
    }

    auto t_utf8 = config_node[toml::path("stdout_utf8")];
    bool utf8 = false;
    if (t_utf8) {
        if (!t_utf8.is_boolean()) {
            spdlog::error("Error reading 'stdout_utf8'");
            std::exit(1);
        }
        utf8 = **t_utf8.as_boolean();
#ifdef _WIN32
        spdlog::info(" stdout_utf8: {}", utf8);
#else
        spdlog::warn("'stdout_utf8' has no effect on Linux and MacOS.");
#endif
    }

    std::unordered_map<std::string, std::string> env;
    auto t_env = config_node[toml::path("env")];
    if (t_env) {
        if (!t_env.is_table()) {
            spdlog::error("Error reading 'env'");
            std::exit(1);
        }
        spdlog::info(" env:");
        for (auto &e : *t_env.as_table()) {
            auto key = e.first.str();
            if (!e.second.is_string()) {
                spdlog::error("Error reading env: '{}'", key);
                std::exit(1);
            }
            auto val = **e.second.as_string();
            env.emplace(key, val);
            spdlog::info("  {} = '{}'", key, val);
        }
    }
    return std::make_shared<Process>(name, exec, workdir, capture, utf8, env);
}

std::vector<std::shared_ptr<Command>> parseToml(webcface::Client &wcli,
                                                toml::parse_result &config) {
    std::vector<std::shared_ptr<Command>> commands;
    if (!config["command"].is_array()) {
        spdlog::error("No commands specified in config.");
        std::exit(1);
    }
    auto config_commands = config["command"].as_array();
    for (auto &&config_node : *config_commands) {
        auto start_p = parseTomlProcess(config_node, "");

        Command::StopOption stop_p = 2;
        auto t_stop = config_node[toml::path("stop")];
        if (t_stop) {
            if (t_stop.is_boolean()) {
                if (**t_stop.as_boolean()) {
                    stop_p.emplace<2>(2);
#ifdef _WIN32
                    spdlog::info(" stop: TerminateProcess");
#else
                    spdlog::info(" stop: signal {}", 2);
#endif
                } else {
                    spdlog::info(" stop: disabled");
                    stop_p.emplace<0>(std::nullopt);
                }
            } else if (t_stop.is_number()) {
                stop_p.emplace<2>(**t_stop.as_integer());
#ifdef _WIN32
                spdlog::info(" stop: TerminateProcess");
#else
                spdlog::info(" stop: signal {}", **t_stop.as_integer());
#endif
            } else if (t_stop.is_table()) {
                auto tb_stop = *t_stop.as_table();
                auto t_exec = tb_stop[toml::path("exec")];
                if (t_exec.is_string()) {
                    stop_p.emplace<1>(
                        parseTomlProcess(tb_stop, start_p->name + "/stop"));
                } else {
                    spdlog::error("Error reading 'stop'");
                    std::exit(1);
                }
            } else {
                spdlog::error("Error reading 'stop'");
                std::exit(1);
            }
        }

        auto cmd = std::make_shared<Command>(start_p, stop_p);
        cmd->initFunc(wcli);
        commands.push_back(cmd);
    }
    return commands;
}

void launcherLoop(WebCFace::Client &wcli,
                  const std::vector<std::shared_ptr<Command>> &commands) {
    auto v = wcli.view("launcher");
    for (auto c : commands) {
        v << c->start_p->name << " ";
        auto start = webcface::button("start", c->start_f);
        auto stop = webcface::button("stop", c->stop_f);
        if (c->start_p->is_running()) {
            // todo: button.disable がほしい
            start.bgColor(WebCFace::ViewColor::gray);
            stop.bgColor(WebCFace::ViewColor::orange);
        } else {
            start.bgColor(WebCFace::ViewColor::green);
            stop.bgColor(WebCFace::ViewColor::gray);
        }
        v << start;
        if (c->stop_p.index() != 0) {
            v << stop;
        }
        if (!c->start_p->is_running() && c->start_p->exit_status != 0) {
            v << webcface::text("(" + std::to_string(c->start_p->exit_status) +
                                ") ")
                     .textColor(WebCFace::ViewColor::red);
        }
        if (!c->start_p->is_running() &&
            (c->start_p->exit_status != 0 ||
             c->start_p->capture_stdout == CaptureMode::always)) {
            std::string logs = c->start_p->logs;
            if (!logs.empty()) {
                v << webcface::button("Clear Logs",
                                      [c] { c->start_p->logs.clear(); })
                         .bgColor(webcface::ViewColor::cyan)
                  << std::endl;
                for (int i;
                     (i = logs.find_first_of("\n")) != std::string::npos;) {
                    v << "　　" << logs.substr(0, i) << std::endl;
                    logs = logs.substr(i + 1);
                }
                if (!logs.empty()) {
                    v << "　　" << logs << std::endl;
                }
            } else {
                v << std::endl;
            }
        } else {
            v << std::endl;
        }
    }
    v.sync();
    wcli.sync();
}


}
WEBCFACE_NS_END

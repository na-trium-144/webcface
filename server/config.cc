#include "./config.h"
#include <iostream>
#include <sstream>
#include <toml++/toml.hpp>

static inline std::string tomlSourceInfo(const toml::source_region &src) {
    std::stringstream ss;
    ss << src;
    return ss.str();
}

ServerConfig parseConfig(const std::string &toml_path, bool use_stdin){
    ServerConfig config_ret;
    toml::parse_result config;
    if (use_stdin) {
        std::string config_str = "";
        while (!std::cin.eof()) {
            std::string input;
            std::getline(std::cin, input);
            config_str += input + "\n";
        }
        config = toml::parse(config_str);
    } else {
        try {
            config = toml::parse_file(toml_path);
        } catch (const toml::parse_error &e) {
            spdlog::error("Error reading config file {}: {} ({})", toml_path,
                          std::string(e.description()),
                          tomlSourceInfo(e.source()));
            return config_ret;
        } catch (const std::exception &e) {
            spdlog::error("Error reading config file {}: {}", toml_path,
                          e.what());
            return config_ret;
        }
    }
    // if (wcli_name.empty()) {
    //     wcli_name = config["init"]["name"].value_or("webcface-launcher");
    // }
    // if (wcli_host.empty()) {
    //     wcli_host = config["init"]["address"].value_or("127.0.0.1");
    // }
    config_ret.port = config["init"]["port"].value_or(0);

    config.commands = webcface::launcher::parseToml(wcli, config);

}
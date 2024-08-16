#include "./config.h"
#include "webcface/launcher/launcher.h"
#include <iostream>
#include <sstream>
#include <toml++/toml.hpp>

static inline std::string tomlSourceInfo(const toml::source_region &src) {
    std::stringstream ss;
    ss << src;
    return ss.str();
}

toml::parse_result parseToml(const std::string &toml_path, bool use_stdin){
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
        } catch (const std::exception &e) {
            spdlog::error("Error reading config file {}: {}", toml_path,
                          e.what());
        }
    }
    return config;
}

ServerConfig parseConfig(toml::parse_result &config){
    ServerConfig config_ret;
    // if (wcli_name.empty()) {
    //     wcli_name = config["init"]["name"].value_or("webcface-launcher");
    // }
    // if (wcli_host.empty()) {
    //     wcli_host = config["init"]["address"].value_or("127.0.0.1");
    // }
    config_ret.port = config["init"]["port"].value_or(0);

    config_ret.commands = webcface::launcher::parseToml(config);

    return config_ret;
}

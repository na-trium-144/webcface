#pragma once
#include <string>
#include <webcface/launcher/command.h>
#include <toml++/toml.hpp>

struct ServerConfig{
    int port = 0;
    std::vector<std::shared_ptr<webcface::launcher::Command>> commands;
    ServerConfig() = default;
};

toml::parse_result parseToml(const std::string &toml_path, bool use_stdin);
ServerConfig parseConfig(toml::parse_result &config);

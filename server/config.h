#pragma once
#include <string>
#include <webcface/launcher/command.h>

struct ServerConfig{
    int port = 0;
    std::vector<std::shared_ptr<webcface::launcher::Command>> commands;
    ServerConfig() = default;
};

ServerConfig parseConfig(const std::string &toml_path, bool use_stdin);

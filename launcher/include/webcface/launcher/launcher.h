#pragma once
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif
#include "./command.h"
#include <toml++/toml.hpp>
#include <memory>
#include <vector>

WEBCFACE_NS_BEGIN
namespace launcher {

std::shared_ptr<Process> parseTomlProcess(toml::node &config_node,
                                          const std::string &default_name);
std::vector<std::shared_ptr<Command>> parseToml(webcface::Client &wcli,
                                                toml::parse_result &config);
void launcherLoop(webcface::Client &wcli,
                  const std::vector<std::shared_ptr<Command>> &commands);

}
WEBCFACE_NS_END

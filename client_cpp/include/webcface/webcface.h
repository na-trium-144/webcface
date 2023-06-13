#pragma once
#include <string>
namespace WebCFace {
void init(const std::string &name, const std::string &host = "127.0.0.1",
          int port = 80);
}
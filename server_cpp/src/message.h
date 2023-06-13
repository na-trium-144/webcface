#pragma once
#include <msgpack.hpp>
#include <string>

namespace WebCFace {
namespace Message {
enum class MessageKind {
    name = 0,
};

using Name = std::string;

} // namespace Message
} // namespace WebCFace
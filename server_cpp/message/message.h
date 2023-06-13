#pragma once
#include <msgpack.hpp>
#include <string>
#include <utility>
#include <any>
namespace WebCFace::Message {
enum class MessageKind {
    name = 0,
};

using Name = std::string;

std::pair<MessageKind, std::any> unpack(const std::string &message);
std::string pack(MessageKind kind, std::any obj);

} // namespace WebCFace::Message

#pragma once
#include <msgpack.hpp>
#include <string>
#include <utility>
#include <any>
namespace WebCFace::Message {
enum class MessageKind {
    name = 0,
    value = 1,
};

template <MessageKind k>
struct MessageBase {
    MessageKind kind = k;
};
struct Name : public MessageBase<MessageKind::name> {
    std::string name;
    Name(){}
    Name(const std::string& name): name(name){}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name));
};
struct Value : public MessageBase<MessageKind::value> {
    std::string name;
    double data;
    Value(){}
    Value(const std::string& name, double data): name(name), data(data){}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name), MSGPACK_NVP("d", data));
};

std::pair<MessageKind, std::any> unpack(const std::string &message);

template <typename T>
std::string pack(T obj) {
    msgpack::type::tuple<int, T> src(static_cast<int>(obj.kind), obj);
    std::stringstream buffer;
    msgpack::pack(buffer, src);
    buffer.seekg(0);
    return buffer.str();
}

} // namespace WebCFace::Message

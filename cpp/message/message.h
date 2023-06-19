#pragma once
#include <msgpack.hpp>
#include <string>
#include <utility>
#include <any>
namespace WebCFace::Message {
enum class MessageKind {
    value = 0,
    text = 1,
    recv = 50, // 50〜
    subscribe = 100, // 100〜
    // 150〜: other
    name = 150,
};
inline constexpr MessageKind kind_subscribe(MessageKind k){
    return static_cast<MessageKind>(static_cast<int>(k) + static_cast<int>(MessageKind::subscribe));
}
inline constexpr MessageKind kind_recv(MessageKind k){
    return static_cast<MessageKind>(static_cast<int>(k) + static_cast<int>(MessageKind::recv));
}

template <MessageKind k>
struct MessageBase {
    static constexpr MessageKind kind = k;
};
struct Name : public MessageBase<MessageKind::name> {
    std::string name;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name));
};
struct Value : public MessageBase<MessageKind::value> {
    std::string name;
    using DataType = double;
    DataType data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name), MSGPACK_NVP("d", data));
};
struct Text : public MessageBase<MessageKind::text> {
    std::string name;
    using DataType = std::string;
    DataType data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("n", name), MSGPACK_NVP("d", data));
};
template <typename T>
struct Recv: public MessageBase<kind_recv(T::kind)>{
    std::string from, name;
    typename T::DataType data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", from), MSGPACK_NVP("n", name), MSGPACK_NVP("d", data));
};
template <typename T>
struct Subscribe: public MessageBase<kind_subscribe(T::kind)>{
    std::string from, name;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", from), MSGPACK_NVP("n", name));
};


std::pair<MessageKind, std::any> unpack(const std::string &message);

template <typename T>
std::string pack(T obj) {
    msgpack::type::tuple<int, T> src(static_cast<int>(T::kind), obj);
    std::stringstream buffer;
    msgpack::pack(buffer, src);
    buffer.seekg(0);
    return buffer.str();
}

} // namespace WebCFace::Message

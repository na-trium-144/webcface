#include "message.h"
namespace WebCFace::Message {
std::pair<MessageKind, std::any> unpack(const std::string &message) {
    // Default construct msgpack::object_handle
    msgpack::object_handle result;

    // Pass the msgpack::object_handle
    unpack(result, message.c_str(), message.size());
    // Get msgpack::object from msgpack::object_handle (shallow copy)
    msgpack::object obj(result.get());
    // Get msgpack::zone from msgpack::object_handle (move)
    // msgpack::unique_ptr<msgpack::zone> z(result.zone());

    auto kind = static_cast<MessageKind>(obj.via.array.ptr[0].as<int>());
    std::any obj_u;
    switch (kind) {
    case MessageKind::name:
        obj_u = obj.via.array.ptr[1].as<Message::Name>();
        break;
    case MessageKind::value:
        obj_u = obj.via.array.ptr[1].as<Message::Value>();
        break;
    }
    return std::make_pair(kind, obj_u);
}

} // namespace WebCFace::Message
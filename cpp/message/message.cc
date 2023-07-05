#include "message.h"
#include <iostream>
namespace WebCFace::Message {
std::pair<MessageKind, std::any> unpack(const std::string &message) {
    try {
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
        switch (kind) {
        case MessageKind::name:
            obj_u = obj.via.array.ptr[1].as<Name>();
            break;
        case MessageKind::call:
            obj_u = obj.via.array.ptr[1].as<Call>();
            break;
        case MessageKind::call_response:
            obj_u = obj.via.array.ptr[1].as<CallResponse>();
            break;
        case MessageKind::entry:
            obj_u = obj.via.array.ptr[1].as<Entry>();
            break;
        case MessageKind::value:
            obj_u = obj.via.array.ptr[1].as<Value>();
            break;
        case MessageKind::text:
            obj_u = obj.via.array.ptr[1].as<Text>();
            break;
        case kind_recv(MessageKind::value):
            obj_u = obj.via.array.ptr[1].as<Recv<Value>>();
            break;
        case kind_recv(MessageKind::text):
            obj_u = obj.via.array.ptr[1].as<Recv<Text>>();
            break;
        case kind_subscribe(MessageKind::value):
            obj_u = obj.via.array.ptr[1].as<Subscribe<Value>>();
            break;
        case kind_subscribe(MessageKind::text):
            obj_u = obj.via.array.ptr[1].as<Subscribe<Text>>();
            break;
        default:
            break;
        }
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
        return std::make_pair(kind, obj_u);
    } catch (const msgpack::type_error &) {
        std::cerr << "unpack error: " << std::hex;
        for (int i = 0; i < message.size(); i++) {
            std::cerr << std::setw(3) << message[i];
        }
        std::cerr << std::dec << std::endl;
        return std::make_pair(MessageKind::unknown, 0);
    }
}

} // namespace WebCFace::Message
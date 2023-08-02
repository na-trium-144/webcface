#include "message.h"
#include <iostream>
namespace WebCFace::Message {
void printMsg(const std::string &message) {
    std::cerr << std::hex;
    for (int i = 0; i < message.size(); i++) {
        std::cerr << std::setw(3) << static_cast<int>(message[i] & 0xff);
    }
    std::cerr << std::dec << std::endl;
    for (int i = 0; i < message.size(); i++) {
        std::cerr << message[i];
    }
    std::cerr << std::endl;
}
std::pair<MessageKind, std::any> unpack(const std::string &message) {
    if (message.size() == 0) {
        return std::make_pair(MessageKind::unknown, 0);
    }
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
        switch (kind) {

#define MSG_PARSE(kind, type)                                                  \
    case MessageKind::kind:                                                    \
        obj_u = obj.via.array.ptr[1].as<type>();                               \
        break;

#define MSG_PARSE_DATA(kind, type)                                             \
    MSG_PARSE(kind, type)                                                      \
    case kind_recv(MessageKind::kind):                                         \
        static_assert(MessageKind::kind < MessageKind::recv &&                 \
                      MessageKind::kind < MessageKind::subscribe);             \
        obj_u = obj.via.array.ptr[1].as<Recv<type>>();                         \
        break;                                                                 \
    case kind_subscribe(MessageKind::kind):                                    \
        obj_u = obj.via.array.ptr[1].as<Subscribe<type>>();                    \
        break;

            MSG_PARSE(name, Name)
            MSG_PARSE(call, Call)
            MSG_PARSE(call_response, CallResponse)
            MSG_PARSE(entry, Entry)
            MSG_PARSE_DATA(value, Value)
            MSG_PARSE_DATA(text, Text)
            MSG_PARSE(func_info, FuncInfo)

#undef MSG_PARSE_DATA
#undef MSG_PARSE
        default:
            break;
        }
#pragma GCC diagnostic pop

        // printMsg(message);
        return std::make_pair(kind, obj_u);
    } catch (const std::exception &e) {
        std::cerr << "unpack error: " << e.what() << std::endl;
        printMsg(message);
        return std::make_pair(MessageKind::unknown, 0);
    }
}

} // namespace WebCFace::Message
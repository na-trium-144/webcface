#include "message.h"
#include <iostream>
namespace WebCFace::Message {
void printMsg(const std::string &message) {
    std::cerr << std::hex;
    for (int i = 0; i < message.size(); i++) {
        std::cerr << std::setw(3) << static_cast<int>(message[i] & 0xff);
    }
    std::cerr << std::dec << std::endl;
    // for (int i = 0; i < message.size(); i++) {
    //     std::cerr << message[i];
    // }
    // std::cerr << std::endl;
}
std::vector<std::pair<int, std::any>> unpack(const std::string &message) {
    if (message.size() == 0) {
        return std::vector<std::pair<int, std::any>>{};
    }
    try {
        msgpack::object_handle result;
        unpack(result, message.c_str(), message.size());
        msgpack::object obj(result.get());
        // msgpack::unique_ptr<msgpack::zone> z(result.zone());

        if (obj.type != msgpack::type::ARRAY || obj.via.array.size % 2 != 0) {
            throw msgpack::type_error();
        }
        std::vector<std::pair<int, std::any>> ret(obj.via.array.size / 2);
        for (std::size_t i = 0; i < obj.via.array.size; i += 2) {
            auto kind = obj.via.array.ptr[i].as<int>();
            std::any obj_u;
            switch (kind) {

#define MSG_PARSE(kind, type)                                                  \
    case MessageKind::kind:                                                    \
        obj_u = obj.via.array.ptr[i + 1].as<type>();                           \
        break;

#define MSG_PARSE_DATA(kind, type)                                             \
    MSG_PARSE(kind, type)                                                      \
    case MessageKind::entry + MessageKind::kind:                               \
        static_assert(MessageKind::kind < MessageKind::entry &&                \
                      MessageKind::kind < MessageKind::req &&                  \
                      MessageKind::kind < MessageKind::res);                   \
        obj_u = obj.via.array.ptr[i + 1].as<Entry<type>>();                    \
        break;                                                                 \
    case MessageKind::req + MessageKind::kind:                                 \
        obj_u = obj.via.array.ptr[i + 1].as<Req<type>>();                      \
        break;                                                                 \
    case MessageKind::res + MessageKind::kind:                                 \
        obj_u = obj.via.array.ptr[i + 1].as<Res<type>>();                      \
        break;

                MSG_PARSE(sync_init, SyncInit)
                MSG_PARSE(call, Call)
                MSG_PARSE(call_response, CallResponse)
                MSG_PARSE(call_result, CallResult)
                MSG_PARSE_DATA(value, Value)
                MSG_PARSE_DATA(text, Text)
                MSG_PARSE_DATA(view, View)
                MSG_PARSE(log, Log)
                MSG_PARSE(log_req, LogReq)
                MSG_PARSE(func_info, FuncInfo)
                MSG_PARSE(sync, Sync)

#undef MSG_PARSE_DATA
#undef MSG_PARSE
            default:
                break;
            }

            // printMsg(message);
            ret[i / 2] = std::make_pair(kind, obj_u);
        }
        return ret;
    } catch (const std::exception &e) {
        std::cerr << "unpack error: " << e.what() << std::endl;
        printMsg(message);
        return std::vector<std::pair<int, std::any>>{};
    }
}

} // namespace WebCFace::Message
#include "message.h"
#include <sstream>

namespace WebCFace::Message {
void printMsg(const std::shared_ptr<spdlog::logger> &logger,
              const std::string &message) {
    std::stringstream ss;
    ss << "message: " << std::hex;
    for (int i = 0; i < message.size(); i++) {
        ss << std::setw(3) << static_cast<int>(message[i] & 0xff);
    }
    logger->debug(ss.str());
    // for (int i = 0; i < message.size(); i++) {
    //     std::cerr << message[i];
    // }
    // std::cerr << std::endl;
}
std::vector<std::pair<int, std::any>>
unpack(const std::string &message,
       const std::shared_ptr<spdlog::logger> &logger) {
    if (message.size() == 0) {
        return std::vector<std::pair<int, std::any>>{};
    }
    msgpack::object_handle result;
    unpack(result, message.c_str(), message.size());
    msgpack::object obj(result.get());
    // msgpack::unique_ptr<msgpack::zone> z(result.zone());

    try {
        if (obj.type != msgpack::type::ARRAY || obj.via.array.size % 2 != 0) {
            logger->error("unpack error: invalid array length");
            return std::vector<std::pair<int, std::any>>{};
        }
        std::vector<std::pair<int, std::any>> ret(obj.via.array.size / 2);
        for (std::size_t i = 0; i < obj.via.array.size; i += 2) {
            auto kind = obj.via.array.ptr[i].as<int>();
            std::any obj_u;
            switch (kind) {

#define MSG_PARSE(kind, type)                                                  \
    case MessageKind::kind:                                                    \
        try {                                                                  \
            obj_u = obj.via.array.ptr[i + 1].as<type>();                       \
        } catch (const std::exception &e) {                                    \
            logger->error("unpack error: {}, index={}, kind={}", e.what(),     \
                          i + 1, MessageKind::kind);                           \
            printMsg(logger, message);                                         \
            continue;                                                          \
        }                                                                      \
        break;

#define MSG_PARSE_DATA(kind, type)                                             \
    MSG_PARSE(kind, type)                                                      \
    case MessageKind::entry + MessageKind::kind:                               \
        static_assert(MessageKind::kind < MessageKind::entry &&                \
                      MessageKind::kind < MessageKind::req &&                  \
                      MessageKind::kind < MessageKind::res);                   \
        try {                                                                  \
            obj_u = obj.via.array.ptr[i + 1].as<Entry<type>>();                \
        } catch (const std::exception &e) {                                    \
            logger->error("unpack error: {}, index={}, kind={}", e.what(),     \
                          i + 1, MessageKind::entry + MessageKind::kind);      \
            printMsg(logger, message);                                         \
            continue;                                                          \
        }                                                                      \
        break;                                                                 \
    case MessageKind::req + MessageKind::kind:                                 \
        try {                                                                  \
            obj_u = obj.via.array.ptr[i + 1].as<Req<type>>();                  \
        } catch (const std::exception &e) {                                    \
            logger->error("unpack error: {}, index={}, kind={}", e.what(),     \
                          i + 1, MessageKind::req + MessageKind::kind);        \
            printMsg(logger, message);                                         \
            continue;                                                          \
        }                                                                      \
        break;                                                                 \
    case MessageKind::res + MessageKind::kind:                                 \
        try {                                                                  \
            obj_u = obj.via.array.ptr[i + 1].as<Res<type>>();                  \
        } catch (const std::exception &e) {                                    \
            logger->error("unpack error: {}, index={}, kind={}", e.what(),     \
                          i + 1, MessageKind::res + MessageKind::kind);        \
            printMsg(logger, message);                                         \
            continue;                                                          \
        }                                                                      \
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
                MSG_PARSE(svr_version, SvrVersion)
                MSG_PARSE(ping, Ping)
                MSG_PARSE(ping_status, PingStatus)
                MSG_PARSE(ping_status_req, PingStatusReq)

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
        logger->error("unpack error: {}", e.what());
        printMsg(logger, message);
        return std::vector<std::pair<int, std::any>>{};
    }
} // namespace WebCFace::Message

} // namespace WebCFace::Message
#include "webcface/message/message.h"
#include <sstream>

WEBCFACE_NS_BEGIN
namespace Message {
template struct WEBCFACE_DLL_INSTANCE_DEF Req<Value>;
template struct WEBCFACE_DLL_INSTANCE_DEF Req<Text>;
template struct WEBCFACE_DLL_INSTANCE_DEF Req<View>;
template struct WEBCFACE_DLL_INSTANCE_DEF Req<Canvas2D>;
template struct WEBCFACE_DLL_INSTANCE_DEF Req<Canvas3D>;
template struct WEBCFACE_DLL_INSTANCE_DEF Req<RobotModel>;
template struct WEBCFACE_DLL_INSTANCE_DEF Entry<Value>;
template struct WEBCFACE_DLL_INSTANCE_DEF Entry<Text>;
template struct WEBCFACE_DLL_INSTANCE_DEF Entry<View>;
template struct WEBCFACE_DLL_INSTANCE_DEF Entry<Canvas2D>;
template struct WEBCFACE_DLL_INSTANCE_DEF Entry<Image>;
template struct WEBCFACE_DLL_INSTANCE_DEF Entry<Canvas3D>;
template struct WEBCFACE_DLL_INSTANCE_DEF Entry<RobotModel>;

static void printMsg(const std::shared_ptr<spdlog::logger> &logger,
                     const std::string &message) {
    std::stringstream ss;
    ss << "message: " << std::hex;
    for (std::size_t i = 0; i < message.size(); i++) {
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
    try {
        msgpack::object_handle result;
        unpack(result, message.c_str(), message.size());
        msgpack::object obj(result.get());
        // msgpack::unique_ptr<msgpack::zone> z(result.zone());

        if (obj.type != msgpack::type::ARRAY || obj.via.array.size % 2 != 0) {
            logger->error("unpack error: invalid array length");
            return std::vector<std::pair<int, std::any>>{};
        }
        std::vector<std::pair<int, std::any>> ret;
        for (std::size_t i = 0; i < obj.via.array.size; i += 2) {
            auto kind = obj.via.array.ptr[i].as<int>();
            std::any obj_u;
            switch (kind) {

#define MSG_PARSE(type)                                                        \
    case type::kind:                                                           \
        try {                                                                  \
            obj_u = obj.via.array.ptr[i + 1].as<type>();                       \
        } catch (const std::exception &e) {                                    \
            logger->error("unpack error: {} at index={}, kind={}", e.what(),   \
                          i + 1, static_cast<int>(type::kind));                \
            printMsg(logger, message);                                         \
            continue;                                                          \
        }                                                                      \
        break;

#define MSG_PARSE_DATA(type)                                                   \
    static_assert(type::kind < MessageKind::entry &&                           \
                  type::kind < MessageKind::req &&                             \
                  type::kind < MessageKind::res);                              \
    MSG_PARSE(type)                                                            \
    MSG_PARSE(Entry<type>)                                                     \
    MSG_PARSE(Req<type>)                                                       \
    MSG_PARSE(Res<type>)

                MSG_PARSE(SyncInit)
                MSG_PARSE(Call)
                MSG_PARSE(CallResponse)
                MSG_PARSE(CallResult)
                MSG_PARSE_DATA(Value)
                MSG_PARSE_DATA(Text)
                MSG_PARSE_DATA(View)
                MSG_PARSE_DATA(Image)
                MSG_PARSE_DATA(RobotModel)
                MSG_PARSE_DATA(Canvas3D)
                MSG_PARSE_DATA(Canvas2D)
                MSG_PARSE(Log)
                MSG_PARSE(LogReq)
                MSG_PARSE(FuncInfo)
                MSG_PARSE(Sync)
                MSG_PARSE(SvrVersion)
                MSG_PARSE(Ping)
                MSG_PARSE(PingStatus)
                MSG_PARSE(PingStatusReq)

#undef MSG_PARSE_DATA
#undef MSG_PARSE
            default:
                break;
            }

            // printMsg(message);
            ret.push_back(std::make_pair(kind, obj_u));
        }
        return ret;
    } catch (const std::exception &e) {
        logger->error("unpack error: {}", e.what());
        printMsg(logger, message);
        return std::vector<std::pair<int, std::any>>{};
    }
}
} // namespace Message
WEBCFACE_NS_END

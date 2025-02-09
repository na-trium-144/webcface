#include "webcface/common/internal/message/pack.h"
#include "webcface/common/internal/message/base.h"
#include "webcface/common/internal/message/canvas2d.h"
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/common/internal/message/func.h"
#include "webcface/common/internal/message/image.h"
#include "webcface/common/internal/message/log.h"
#include "webcface/common/internal/message/robot_model.h"
#include "webcface/common/internal/message/sync.h"
#include "webcface/common/internal/message/text.h"
#include "webcface/common/internal/message/value.h"
#include "webcface/common/internal/message/view.h"

WEBCFACE_NS_BEGIN
namespace message {
std::string messageTrace(const std::string &message) {
    std::string message_str;
    for (const auto &c : message) {
        fmt::format_to(std::back_inserter(message_str), "{:02x} ", c);
    }
    return message_str;
}

static std::string objectTypeStr(msgpack::type::object_type type) {
    switch (type) {
    case msgpack::type::NIL:
        return "nil";
    case msgpack::type::BOOLEAN:
        return "boolean";
    case msgpack::type::POSITIVE_INTEGER:
        return "positive integer";
    case msgpack::type::NEGATIVE_INTEGER:
        return "negative integer";
    case msgpack::type::FLOAT32:
        return "float32";
    case msgpack::type::FLOAT64:
        return "float64";
    case msgpack::type::STR:
        return "string";
    case msgpack::type::BIN:
        return "binary";
    case msgpack::type::ARRAY:
        return "array";
    case msgpack::type::MAP:
        return "map";
    case msgpack::type::EXT:
        return "extension";
    default:
        return "unknown object_type " + std::to_string(static_cast<int>(type));
    }
}
std::vector<std::pair<int, std::shared_ptr<void>>>
unpack(const std::string &message,
       const std::shared_ptr<spdlog::logger> &logger) {
    if (message.size() == 0) {
        return std::vector<std::pair<int, std::shared_ptr<void>>>{};
    }
    try {
        msgpack::object_handle result;
        unpack(result, message.c_str(), message.size());
        msgpack::object obj(result.get());
        // msgpack::unique_ptr<msgpack::zone> z(result.zone());

        if (obj.type != msgpack::type::ARRAY) {
            logger->error("unpack error: array expected, got {}",
                          objectTypeStr(obj.type));
            return std::vector<std::pair<int, std::shared_ptr<void>>>{};
        }
        if (obj.via.array.size % 2 != 0) {
            logger->error("unpack error: array length must be even, got {}",
                          obj.via.array.size);
            return std::vector<std::pair<int, std::shared_ptr<void>>>{};
        }
        std::vector<std::pair<int, std::shared_ptr<void>>> ret;
        for (std::size_t i = 0; i < obj.via.array.size; i += 2) {
            if (obj.via.array.ptr[i].type != msgpack::type::POSITIVE_INTEGER) {
                logger->error("unpack error: message kind as positive integer "
                              "expected, got {}",
                              objectTypeStr(obj.via.array.ptr[i].type));
                return std::vector<std::pair<int, std::shared_ptr<void>>>{};
            }
            auto kind = obj.via.array.ptr[i].as<int>();
            std::shared_ptr<void> obj_u;
            switch (kind) {

#define MSG_PARSE(type)                                                        \
    case type::kind:                                                           \
        try {                                                                  \
            obj_u =                                                            \
                std::make_shared<type>(obj.via.array.ptr[i + 1].as<type>());   \
        } catch (const std::exception &e) {                                    \
            logger->error("unpack error: {} (at index={}, kind={})", e.what(), \
                          i + 1, static_cast<int>(type::kind));                \
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
                MSG_PARSE_DATA(ViewOld)
                MSG_PARSE_DATA(Image)
                MSG_PARSE_DATA(RobotModel)
                MSG_PARSE_DATA(Canvas3D)
                MSG_PARSE_DATA(Canvas3DOld)
                MSG_PARSE_DATA(Canvas2D)
                MSG_PARSE_DATA(Canvas2DOld)
                MSG_PARSE_DATA(Log)
                MSG_PARSE(LogDefault)
                MSG_PARSE(LogReqDefault)
                MSG_PARSE(LogEntryDefault)
                MSG_PARSE(FuncInfo)
                MSG_PARSE(Sync)
                MSG_PARSE(SyncInitEnd)
                MSG_PARSE(Ping)
                MSG_PARSE(PingStatus)
                MSG_PARSE(PingStatusReq)
                MSG_PARSE(Closed)

#undef MSG_PARSE_DATA
#undef MSG_PARSE
            default:
                break;
            }

            // printMsg(message);
            ret.push_back(std::make_pair(kind, std::move(obj_u)));
        }
        return ret;
    } catch (const std::exception &e) {
        logger->error("unpack error: {}", e.what());
        return std::vector<std::pair<int, std::shared_ptr<void>>>{};
    }
}

SharedString Log::defaultLogName() {
    static SharedString default_name = SharedString::fromU8String("default");
    return default_name;
}

} // namespace message
WEBCFACE_NS_END

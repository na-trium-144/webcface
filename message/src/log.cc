#include "webcface/message/log.h"

WEBCFACE_NS_BEGIN
namespace message {

SharedString Log::defaultLogName() {
    static SharedString default_name = SharedString::fromU8String("default");
    return default_name;
}

std::shared_ptr<void>
unpackLogSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::log:
        return unpackSingleT<Log>(obj, index, logger);
    case MessageKind::log + MessageKind::req:
        return unpackSingleT<Req<Log>>(obj, index, logger);
    case MessageKind::log + MessageKind::res:
        return unpackSingleT<Res<Log>>(obj, index, logger);
    case MessageKind::log + MessageKind::entry:
        return unpackSingleT<Entry<Log>>(obj, index, logger);
    case MessageKind::log_default:
        return unpackSingleT<LogDefault>(obj, index, logger);
    case MessageKind::log_entry_default:
        return unpackSingleT<LogEntryDefault>(obj, index, logger);
    case MessageKind::log_req_default:
        return unpackSingleT<LogReqDefault>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

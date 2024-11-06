#include "webcface/message/value.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackValueSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::value:
        return unpackSingleT<Value>(obj, index, logger);
    case MessageKind::value + MessageKind::req:
        return unpackSingleT<Req<Value>>(obj, index, logger);
    case MessageKind::value + MessageKind::res:
        return unpackSingleT<Res<Value>>(obj, index, logger);
    case MessageKind::value + MessageKind::entry:
        return unpackSingleT<Entry<Value>>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

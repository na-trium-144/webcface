#include "webcface/message/text.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackTextSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::text:
        return unpackSingleT<Text>(obj, index, logger);
    case MessageKind::text + MessageKind::req:
        return unpackSingleT<Req<Text>>(obj, index, logger);
    case MessageKind::text + MessageKind::res:
        return unpackSingleT<Res<Text>>(obj, index, logger);
    case MessageKind::text + MessageKind::entry:
        return unpackSingleT<Entry<Text>>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

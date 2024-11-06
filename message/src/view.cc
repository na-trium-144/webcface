#include "webcface/message/view.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackViewSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::view:
        return unpackSingleT<View>(obj, index, logger);
    case MessageKind::view + MessageKind::req:
        return unpackSingleT<Req<View>>(obj, index, logger);
    case MessageKind::view + MessageKind::res:
        return unpackSingleT<Res<View>>(obj, index, logger);
    case MessageKind::view + MessageKind::entry:
        return unpackSingleT<Entry<View>>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

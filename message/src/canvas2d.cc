#include "webcface/message/canvas2d.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackCanvas2DSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::canvas2d:
        return unpackSingleT<Canvas2D>(obj, index, logger);
    case MessageKind::canvas2d + MessageKind::req:
        return unpackSingleT<Req<Canvas2D>>(obj, index, logger);
    case MessageKind::canvas2d + MessageKind::res:
        return unpackSingleT<Res<Canvas2D>>(obj, index, logger);
    case MessageKind::canvas2d + MessageKind::entry:
        return unpackSingleT<Entry<Canvas2D>>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

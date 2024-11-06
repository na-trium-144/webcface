#include "webcface/message/canvas3d.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackCanvas3DSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::canvas3d:
        return unpackSingleT<Canvas3D>(obj, index, logger);
    case MessageKind::canvas3d + MessageKind::req:
        return unpackSingleT<Req<Canvas3D>>(obj, index, logger);
    case MessageKind::canvas3d + MessageKind::res:
        return unpackSingleT<Res<Canvas3D>>(obj, index, logger);
    case MessageKind::canvas3d + MessageKind::entry:
        return unpackSingleT<Entry<Canvas3D>>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

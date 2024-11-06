#include "webcface/message/image.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackImageSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::image:
        return unpackSingleT<Image>(obj, index, logger);
    case MessageKind::image + MessageKind::req:
        return unpackSingleT<Req<Image>>(obj, index, logger);
    case MessageKind::image + MessageKind::res:
        return unpackSingleT<Res<Image>>(obj, index, logger);
    case MessageKind::image + MessageKind::entry:
        return unpackSingleT<Entry<Image>>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

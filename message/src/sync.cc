#include "webcface/message/sync.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackSyncSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::sync:
        return unpackSingleT<Sync>(obj, index, logger);
    case MessageKind::sync_init:
        return unpackSingleT<SyncInit>(obj, index, logger);
    case MessageKind::sync_init_end:
        return unpackSingleT<SyncInitEnd>(obj, index, logger);
    case MessageKind::ping:
        return unpackSingleT<Ping>(obj, index, logger);
    case MessageKind::ping_status:
        return unpackSingleT<PingStatus>(obj, index, logger);
    case MessageKind::ping_status_req:
        return unpackSingleT<PingStatusReq>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

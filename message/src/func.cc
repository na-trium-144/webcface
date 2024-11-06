#include "webcface/message/func.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackFuncSingle(int kind, const msgpack::object &obj, std::size_t index,
                 const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::call:
        return unpackSingleT<Call>(obj, index, logger);
    case MessageKind::call_response:
        return unpackSingleT<CallResponse>(obj, index, logger);
    case MessageKind::call_result:
        return unpackSingleT<CallResult>(obj, index, logger);
    case MessageKind::func_info:
        return unpackSingleT<FuncInfo>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

#include "webcface/message/robot_model.h"

WEBCFACE_NS_BEGIN
namespace message {

std::shared_ptr<void>
unpackRobotModelSingle(int kind, const msgpack::object &obj, std::size_t index,
                       const std::shared_ptr<spdlog::logger> &logger) {
    switch (kind) {
    case MessageKind::robot_model:
        return unpackSingleT<RobotModel>(obj, index, logger);
    case MessageKind::robot_model + MessageKind::req:
        return unpackSingleT<Req<RobotModel>>(obj, index, logger);
    case MessageKind::robot_model + MessageKind::res:
        return unpackSingleT<Res<RobotModel>>(obj, index, logger);
    case MessageKind::robot_model + MessageKind::entry:
        return unpackSingleT<Entry<RobotModel>>(obj, index, logger);
    default:
        logger->error("Invalid message Kind {}", kind);
        return nullptr;
    }
}

} // namespace message
WEBCFACE_NS_END

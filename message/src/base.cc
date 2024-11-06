#include "webcface/message/base.h"
#include <sstream>

WEBCFACE_NS_BEGIN
namespace message {
static void printMsg(const std::shared_ptr<spdlog::logger> &logger,
                     const std::string &message) {
    std::stringstream ss;
    ss << "message: " << std::hex;
    for (std::size_t i = 0; i < message.size(); i++) {
        ss << std::setw(3) << static_cast<int>(message[i] & 0xff);
    }
    logger->debug(ss.str());
    // for (int i = 0; i < message.size(); i++) {
    //     std::cerr << message[i];
    // }
    // std::cerr << std::endl;
}
std::vector<std::pair<int, std::shared_ptr<void>>>
unpack(const std::string &message,
       const std::shared_ptr<spdlog::logger> &logger) {
    if (message.size() == 0) {
        return std::vector<std::pair<int, std::shared_ptr<void>>>{};
    }
    try {
        msgpack::object_handle result;
        unpack(result, message.c_str(), message.size());
        msgpack::object obj(result.get());
        // msgpack::unique_ptr<msgpack::zone> z(result.zone());

        if (obj.type != msgpack::type::ARRAY || obj.via.array.size % 2 != 0) {
            logger->error("unpack error: invalid array length");
            return std::vector<std::pair<int, std::shared_ptr<void>>>{};
        }
        std::vector<std::pair<int, std::shared_ptr<void>>> ret;
        for (std::size_t i = 0; i < obj.via.array.size; i += 2) {
            auto kind = obj.via.array.ptr[i].as<int>();
            std::shared_ptr<void> obj_u;
            switch (kind) {
            case MessageKind::sync_init:
            case MessageKind::sync:
            case MessageKind::sync_init_end:
            case MessageKind::ping:
            case MessageKind::ping_status:
            case MessageKind::ping_status_req:
                obj_u = unpackSyncSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::call:
            case MessageKind::call_response:
            case MessageKind::call_result:
            case MessageKind::func_info:
                obj_u = unpackFuncSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::value:
            case MessageKind::value + MessageKind::req:
            case MessageKind::value + MessageKind::res:
            case MessageKind::value + MessageKind::entry:
                obj_u = unpackValueSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::text:
            case MessageKind::text + MessageKind::req:
            case MessageKind::text + MessageKind::res:
            case MessageKind::text + MessageKind::entry:
                obj_u = unpackTextSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::view:
            case MessageKind::view + MessageKind::req:
            case MessageKind::view + MessageKind::res:
            case MessageKind::view + MessageKind::entry:
                obj_u = unpackViewSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::image:
            case MessageKind::image + MessageKind::req:
            case MessageKind::image + MessageKind::res:
            case MessageKind::image + MessageKind::entry:
                obj_u = unpackImageSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::robot_model:
            case MessageKind::robot_model + MessageKind::req:
            case MessageKind::robot_model + MessageKind::res:
            case MessageKind::robot_model + MessageKind::entry:
                obj_u = unpackRobotModelSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::canvas2d:
            case MessageKind::canvas2d + MessageKind::req:
            case MessageKind::canvas2d + MessageKind::res:
            case MessageKind::canvas2d + MessageKind::entry:
                obj_u = unpackCanvas2DSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::canvas3d:
            case MessageKind::canvas3d + MessageKind::req:
            case MessageKind::canvas3d + MessageKind::res:
            case MessageKind::canvas3d + MessageKind::entry:
                obj_u = unpackCanvas3DSingle(kind, obj, i + 1, logger);
                break;
            case MessageKind::log:
            case MessageKind::log + MessageKind::req:
            case MessageKind::log + MessageKind::res:
            case MessageKind::log + MessageKind::entry:
            case MessageKind::log_default:
            case MessageKind::log_entry_default:
            case MessageKind::log_req_default:
                obj_u = unpackLogSingle(kind, obj, i + 1, logger);
                break;
                // default:
                //     break;
            }

            // printMsg(message);
            if (obj_u) {
                ret.push_back(std::make_pair(kind, std::move(obj_u)));
            }
        }
        return ret;
    } catch (const std::exception &e) {
        logger->error("unpack error: {}", e.what());
        printMsg(logger, message);
        return std::vector<std::pair<int, std::shared_ptr<void>>>{};
    }
}

} // namespace message
WEBCFACE_NS_END

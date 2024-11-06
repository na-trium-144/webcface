#include "webcface/internal/client_internal.h"
#include "webcface/message/robot_model.h"
#include "webcface/robot_model.h"
#include "webcface/internal/robot_link_internal.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_robot_model(
    int kind, const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::robot_model + MessageKind::res: {
        auto &r = *static_cast<message::Res<message::RobotModel> *>(obj.get());
        auto links_data = std::make_shared<
            std::vector<std::shared_ptr<internal::RobotLinkData>>>();
        links_data->reserve(r.data.size());
        for (std::size_t i = 0; i < r.data.size(); i++) {
            links_data->push_back(std::make_shared<internal::RobotLinkData>(
                *r.data[i], *links_data));
        }
        onRecvRes(this, r, links_data, this->robot_model_store,
                  this->robot_model_change_event);
        break;
    }
    case MessageKind::entry + MessageKind::robot_model: {
        auto &r = *static_cast<
            webcface::message::Entry<webcface::message::RobotModel> *>(
            obj.get());
        onRecvEntry(this, r, this->robot_model_store,
                    this->robot_model_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_robot_model(
    std::stringstream &buffer, int &len, const SyncDataFirst &data) {
    for (const auto &v : data.robot_model_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::RobotModel>{
                              {}, v.first, v2.first, v2.second});
        }
    }
}
void internal::ClientData::packSyncData_robot_model(
    std::stringstream &buffer, int &len, const SyncDataSnapshot &data) {
    for (const auto &v : data.robot_model_data) {
        std::vector<std::shared_ptr<message::RobotLink>> links;
        links.reserve(v.second->size());
        for (std::size_t i = 0; i < v.second->size(); i++) {
            links.emplace_back(v.second->at(i));
        }
        message::pack(buffer, len, message::RobotModel{v.first, links});
    }
}

WEBCFACE_NS_END

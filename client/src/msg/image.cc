#include "webcface/internal/client_internal.h"
#include "webcface/message/image.h"
#include "webcface/image.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_image(int kind,
                                        const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::image + MessageKind::res: {
        auto &r =
            *static_cast<webcface::message::Res<webcface::message::Image> *>(
                obj.get());
        onRecvRes(this, r, r, this->image_store, this->image_change_event);
        break;
    }
    case MessageKind::entry + MessageKind::image: {
        auto &r =
            *static_cast<webcface::message::Entry<webcface::message::Image> *>(
                obj.get());
        onRecvEntry(this, r, this->image_store, this->image_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_image(std::stringstream &buffer,
                                                   int &len,
                                                   const SyncDataFirst &data) {
    for (const auto &v : data.image_req) {
        for (const auto &v2 : v.second) {
            message::pack(buffer, len,
                          message::Req<message::Image>{
                              v.first, v2.first, v2.second,
                              data.image_req_info.at(v.first).at(v2.first)});
        }
    }
}
void internal::ClientData::packSyncData_image(std::stringstream &buffer,
                                              int &len,
                                              const SyncDataSnapshot &data) {
    for (const auto &v : data.image_data) {
        message::pack(buffer, len,
                      message::Image{v.first, v.second.toMessage()});
    }
}

WEBCFACE_NS_END

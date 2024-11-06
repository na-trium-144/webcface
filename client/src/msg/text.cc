#include "webcface/internal/client_internal.h"
#include "webcface/message/text.h"
#include "webcface/text.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_text(int kind,
                                       const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::text + MessageKind::res: {
        auto &r =
            *static_cast<webcface::message::Res<webcface::message::Text> *>(
                obj.get());
        onRecvRes(this, r, r.data, this->text_store, this->text_change_event);
        break;
    }
    case MessageKind::entry + MessageKind::text: {
        auto &r =
            *static_cast<webcface::message::Entry<webcface::message::Text> *>(
                obj.get());
        onRecvEntry(this, r, this->text_store, this->text_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_text(std::stringstream &buffer,
                                                  int &len,
                                                  const SyncDataFirst &data) {
    for (const auto &v : data.text_req) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::Text>{{}, v.first, v2.first, v2.second});
        }
    }
}
void internal::ClientData::packSyncData_text(std::stringstream &buffer,
                                             int &len,
                                             const SyncDataSnapshot &data) {
    for (const auto &v : data.text_data) {
        message::pack(buffer, len, message::Text{{}, v.first, v.second});
    }
}

WEBCFACE_NS_END

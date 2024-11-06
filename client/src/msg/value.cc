#include "webcface/internal/client_internal.h"
#include "webcface/message/value.h"
#include "webcface/value.h"

WEBCFACE_NS_BEGIN

void internal::ClientData::onRecv_value(int kind,
                                        const std::shared_ptr<void> &obj) {
    namespace MessageKind = webcface::message::MessageKind;
    switch (kind) {
    case MessageKind::value + MessageKind::res: {
        auto &r =
            *static_cast<webcface::message::Res<webcface::message::Value> *>(
                obj.get());
        onRecvRes(this, r, r.data, this->value_store, this->value_change_event);
        break;
    }
    case MessageKind::entry + MessageKind::value: {
        auto &r =
            *static_cast<webcface::message::Entry<webcface::message::Value> *>(
                obj.get());
        onRecvEntry(this, r, this->value_store, this->value_entry_event);
        break;
    }
    default:
        throw std::runtime_error("Invalid message Kind");
    }
}
void internal::ClientData::packSyncDataFirst_value(std::stringstream &buffer,
                                                   int &len,
                                                   const SyncDataFirst &data) {
    for (const auto &v : data.value_req) {
        for (const auto &v2 : v.second) {
            message::pack(
                buffer, len,
                message::Req<message::Value>{{}, v.first, v2.first, v2.second});
        }
    }
}
void internal::ClientData::packSyncData_value(std::stringstream &buffer,
                                              int &len,
                                              const SyncDataSnapshot &data) {
    for (const auto &v : data.value_data) {
        message::pack(buffer, len, message::Value{{}, v.first, v.second});
    }
}

WEBCFACE_NS_END

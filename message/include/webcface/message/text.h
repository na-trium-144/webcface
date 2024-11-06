#pragma once
#include "webcface/message/base.h"

WEBCFACE_NS_BEGIN
namespace message {

struct Text : public MessageBase<MessageKind::text> {
    SharedString field;
    std::shared_ptr<ValAdaptor> data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};
template <>
struct Res<Text> : public MessageBase<MessageKind::text + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::shared_ptr<ValAdaptor> data;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::shared_ptr<ValAdaptor> &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data))
};

} // namespace message
WEBCFACE_NS_END

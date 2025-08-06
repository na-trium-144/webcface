#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include "webcface/common/val_adaptor.h"

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct Text : public MessageBase<MessageKind::text> {
    SharedString field;
    ValAdaptor data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};

template <>
struct Res<Text> : public MessageBase<MessageKind::text + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    ValAdaptor data;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const ValAdaptor &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data))
};

}
WEBCFACE_NS_END

WEBCFACE_MESSAGE_FMT(webcface::message::Text)
WEBCFACE_MESSAGE_FMT(webcface::message::Res<webcface::message::Text>)
WEBCFACE_MESSAGE_FMT(webcface::message::Entry<webcface::message::Text>)
WEBCFACE_MESSAGE_FMT(webcface::message::Req<webcface::message::Text>)

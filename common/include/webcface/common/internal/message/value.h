#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include <vector>

WEBCFACE_NS_BEGIN
namespace message {

struct Value : public MessageBase<MessageKind::value> {
    SharedString field;
    std::shared_ptr<std::vector<double>> data;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};

/*!
 * \brief server->client  Value,Textなどのfieldをreqidに変えただけのもの
 *
 * requestしたフィールドの子フィールドの場合sub_fieldにフィールド名を入れて返す
 * →その場合clientが再度requestを送るはず
 *
 */
template <>
struct Res<Value> : public MessageBase<MessageKind::value + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::shared_ptr<std::vector<double>> data;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::shared_ptr<std::vector<double>> &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data))
};

} // namespace message
WEBCFACE_NS_END

WEBCFACE_MESSAGE_FMT(webcface::message::Value)
WEBCFACE_MESSAGE_FMT(webcface::message::Res<webcface::message::Value>)
WEBCFACE_MESSAGE_FMT(webcface::message::Entry<webcface::message::Value>)
WEBCFACE_MESSAGE_FMT(webcface::message::Req<webcface::message::Value>)

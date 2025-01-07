#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include <vector>
#include <optional>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct ValueShape {
    std::size_t size = 1;
    bool fixed = false;
};
struct Value : public MessageBase<MessageKind::value> {
    SharedString field;
    std::shared_ptr<std::vector<double>> data;
    std::size_t size = 1;
    bool fixed = false;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data),
                       MSGPACK_NVP("s", size), MSGPACK_NVP("x", fixed))
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

template <typename T>
struct Entry;
template <>
struct Entry<Value> : public MessageBase<Value::kind + MessageKind::entry> {
    unsigned int member_id = 0;
    SharedString field;
    std::size_t size = 1;
    bool fixed = false;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("m", member_id), MSGPACK_NVP("f", field),
                       MSGPACK_NVP("s", size), MSGPACK_NVP("x", fixed))
};

} // namespace message
WEBCFACE_NS_END

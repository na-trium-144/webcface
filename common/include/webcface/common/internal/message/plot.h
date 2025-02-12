#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include "webcface/common/val_adaptor.h"
#include <vector>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct PlotSeriesData {
    std::vector<SharedString> value_member, value_field;
    int color = 0, type = 0;
    std::array<double, 4> range;

    bool operator==(const PlotSeriesData &other) const {
        return value_member == other.value_member &&
               value_field == other.value_field && color == other.color;
    }
    bool operator!=(const PlotSeriesData &other) const {
        return !(*this == other);
    }

    MSGPACK_DEFINE_MAP(MSGPACK_NVP("V", value_member),
                       MSGPACK_NVP("v", value_field), MSGPACK_NVP("c", color),
                       MSGPACK_NVP("t", type), MSGPACK_NVP("r", range))
};

struct Plot : public MessageBase<MessageKind::plot> {
    SharedString field;
    std::vector<std::shared_ptr<PlotSeriesData>> data;
    Plot() = default;
    Plot(const SharedString &field,
         const std::vector<std::shared_ptr<PlotSeriesData>> &data)
        : field(field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data))
};

template <>
struct Res<Plot> : public MessageBase<MessageKind::plot + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::vector<std::shared_ptr<PlotSeriesData>> data;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::vector<std::shared_ptr<PlotSeriesData>> &data)
        : req_id(req_id), sub_field(sub_field), data(data) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data))
};

} // namespace message
WEBCFACE_NS_END

WEBCFACE_MESSAGE_FMT(webcface::message::Plot)
WEBCFACE_MESSAGE_FMT(webcface::message::Res<webcface::message::Plot>)
WEBCFACE_MESSAGE_FMT(webcface::message::Entry<webcface::message::Plot>)
WEBCFACE_MESSAGE_FMT(webcface::message::Req<webcface::message::Plot>)

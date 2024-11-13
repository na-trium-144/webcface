#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include "webcface/common/val_adaptor.h"
#include <optional>
#include <unordered_map>

#ifndef MSGPACK_DEFINE_MAP
#define MSGPACK_DEFINE_MAP(...)
#endif

WEBCFACE_NS_BEGIN
namespace message {

struct ViewComponent {
    int type = 0;
    SharedString text;
    std::optional<SharedString> on_click_member, on_click_field;
    std::optional<SharedString> text_ref_member, text_ref_field;
    int text_color = 0, bg_color = 0;
    std::optional<double> min_ = std::nullopt, max_ = std::nullopt,
                          step_ = std::nullopt;
    std::vector<ValAdaptor> option_;
    ViewComponent() = default;
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("t", type), MSGPACK_NVP("x", text),
                       MSGPACK_NVP("L", on_click_member),
                       MSGPACK_NVP("l", on_click_field),
                       MSGPACK_NVP("R", text_ref_member),
                       MSGPACK_NVP("r", text_ref_field),
                       MSGPACK_NVP("c", text_color), MSGPACK_NVP("b", bg_color),
                       MSGPACK_NVP("im", min_), MSGPACK_NVP("ix", max_),
                       MSGPACK_NVP("is", step_), MSGPACK_NVP("io", option_))
};
struct ViewOld : public MessageBase<MessageKind::view_old> {
    SharedString field;
    std::map<std::string, std::shared_ptr<ViewComponent>> data_diff;
    std::size_t length = 0;
    ViewOld() = default;
    ViewOld(const SharedString &field,
            const std::unordered_map<int, std::shared_ptr<ViewComponent>>
                &data_diff,
            std::size_t length)
        : field(field), data_diff(), length(length) {
        for (auto &&vc : data_diff) {
            this->data_diff.emplace(std::to_string(vc.first), vc.second);
        }
    }
    ViewOld(
        const SharedString &field,
        const std::map<std::string, std::shared_ptr<ViewComponent>> &data_diff,
        std::size_t length)
        : field(field), data_diff(data_diff), length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", length))
};
struct View : public MessageBase<MessageKind::view> {
    SharedString field;
    std::map<std::string, std::shared_ptr<ViewComponent>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    View() = default;
    View(const SharedString &field,
         std::map<std::string, std::shared_ptr<ViewComponent>> data_diff,
         std::optional<std::vector<SharedString>> data_ids)
        : field(field), data_diff(std::move(data_diff)),
          data_ids(std::move(data_ids)) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("f", field), MSGPACK_NVP("d", data_diff),
                       MSGPACK_NVP("l", data_ids))
};

template <>
struct Res<ViewOld>
    : public MessageBase<MessageKind::view_old + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::map<std::string, std::shared_ptr<ViewComponent>> data_diff;
    std::size_t length = 0;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::map<std::string, std::shared_ptr<ViewComponent>> &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), data_diff(data_diff),
          length(length) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", length))
};
template <>
struct Res<View> : public MessageBase<MessageKind::view + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    std::map<std::string, std::shared_ptr<ViewComponent>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field,
        const std::map<std::string, std::shared_ptr<ViewComponent>> &data_diff,
        const std::optional<std::vector<SharedString>> &data_ids)
        : req_id(req_id), sub_field(sub_field), data_diff(data_diff),
          data_ids(data_ids) {}
    MSGPACK_DEFINE_MAP(MSGPACK_NVP("i", req_id), MSGPACK_NVP("f", sub_field),
                       MSGPACK_NVP("d", data_diff), MSGPACK_NVP("l", data_ids))
};

}
WEBCFACE_NS_END

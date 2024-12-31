#pragma once
#include "./base.h"
#include "webcface/common/encoding.h"
#include <optional>
#include <unordered_map>
#include <vector>
#include <array>

WEBCFACE_NS_BEGIN
namespace message {

struct Canvas2DComponentData {
    std::int8_t type = 0;
    std::array<double, 2> origin_pos;
    double origin_rot;
    std::int8_t color = 0, fill = 0;
    double stroke_width;
    std::optional<std::int8_t> geometry_type;
    std::vector<double> properties;
    std::optional<SharedString> on_click_member, on_click_field;
    SharedString text;
    Canvas2DComponentData() = default;
    bool operator==(const Canvas2DComponentData &other) const {
        return type == other.type && origin_pos == other.origin_pos &&
               origin_rot == other.origin_rot && color == other.color &&
               fill == other.fill && stroke_width == other.stroke_width &&
               geometry_type == other.geometry_type &&
               properties == other.properties &&
               on_click_member == other.on_click_member &&
               on_click_field == other.on_click_field && text == other.text;
    }
    bool operator!=(const Canvas2DComponentData &other) const {
        return !(*this == other);
    }

    void write(mpack_writer_t *writer) const;
    static Canvas2DComponentData parse(const mpack_node_t &writer);
};
struct Canvas2DData {
    double width = 0, height = 0;
    std::map<std::string, std::shared_ptr<Canvas2DComponentData>> components;
    std::vector<SharedString> data_ids;
    Canvas2DData() = default;
    Canvas2DData(double width, double height)
        : width(width), height(height), components(), data_ids() {}
};

struct Canvas2D : public MessageBase<MessageKind::canvas2d> {
    SharedString field;
    double width, height;
    std::map<std::string, std::shared_ptr<Canvas2DComponentData>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    Canvas2D() = default;
    Canvas2D(
        const SharedString &field, double width, double height,
        std::map<std::string, std::shared_ptr<Canvas2DComponentData>> data_diff,
        std::optional<std::vector<SharedString>> data_ids)
        : field(field), width(width), height(height),
          data_diff(std::move(data_diff)), data_ids(std::move(data_ids)) {}

    void write(mpack_writer_t *writer) const;
    static Canvas2D parse(const mpack_node_t &writer);
};
struct Canvas2DOld : public MessageBase<MessageKind::canvas2d_old> {
    SharedString field;
    double width, height;
    std::map<std::string, std::shared_ptr<Canvas2DComponentData>> data_diff;
    std::size_t length;
    Canvas2DOld() = default;
    Canvas2DOld(
        const SharedString &field, double width, double height,
        const std::unordered_map<int, std::shared_ptr<Canvas2DComponentData>>
            &data_diff,
        std::size_t length)
        : field(field), width(width), height(height), data_diff(),
          length(length) {
        for (const auto &vc : data_diff) {
            this->data_diff.emplace(std::to_string(vc.first), vc.second);
        }
    }
    Canvas2DOld(
        const SharedString &field, double width, double height,
        const std::map<std::string, std::shared_ptr<Canvas2DComponentData>>
            &data_diff,
        std::size_t length)
        : field(field), width(width), height(height), data_diff(data_diff),
          length(length) {}

    void write(mpack_writer_t *writer) const;
    static Canvas2DOld parse(const mpack_node_t &writer);
};
template <>
struct Res<Canvas2DOld>
    : public MessageBase<MessageKind::canvas2d_old + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    double width = 0, height = 0;
    std::map<std::string, std::shared_ptr<Canvas2DComponentData>> data_diff;
    std::size_t length;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field, double width,
        double height,
        const std::map<std::string, std::shared_ptr<Canvas2DComponentData>>
            &data_diff,
        std::size_t length)
        : req_id(req_id), sub_field(sub_field), width(width), height(height),
          data_diff(data_diff), length(length) {}

    void write(mpack_writer_t *writer) const;
    static Res<Canvas2DOld> parse(const mpack_node_t &writer);
};
template <>
struct Res<Canvas2D>
    : public MessageBase<MessageKind::canvas2d + MessageKind::res> {
    unsigned int req_id = 0;
    SharedString sub_field;
    double width = 0, height = 0;
    std::map<std::string, std::shared_ptr<Canvas2DComponentData>> data_diff;
    std::optional<std::vector<SharedString>> data_ids;
    Res() = default;
    Res(unsigned int req_id, const SharedString &sub_field, double width,
        double height,
        const std::map<std::string, std::shared_ptr<Canvas2DComponentData>>
            &data_diff,
        const std::optional<std::vector<SharedString>> &data_ids)
        : req_id(req_id), sub_field(sub_field), width(width), height(height),
          data_diff(data_diff), data_ids(data_ids) {}

    void write(mpack_writer_t *writer) const;
    static Res<Canvas2D> parse(const mpack_node_t &writer);
};

} // namespace message
WEBCFACE_NS_END

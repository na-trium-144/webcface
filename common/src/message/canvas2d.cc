#include "webcface/common/internal/message/canvas2d.h"

WEBCFACE_NS_BEGIN
namespace message {

void Canvas2DComponentData::write(mpack_writer_t *writer) const {
    mpack_build_map(writer);
    mpack_write_cstr(writer, "t");
    mpack_write_i8(writer, type);
    mpack_write_cstr(writer, "op");
    mpack_start_array(writer, 2);
    mpack_write_double(writer, origin_pos[0]);
    mpack_write_double(writer, origin_pos[1]);
    mpack_finish_array(writer);
    mpack_write_cstr(writer, "or");
    mpack_write_double(writer, origin_rot);
    mpack_write_cstr(writer, "c");
    mpack_write_i8(writer, color);
    mpack_write_cstr(writer, "f");
    mpack_write_i8(writer, fill);
    mpack_write_cstr(writer, "s");
    mpack_write_double(writer, stroke_width);
    if (geometry_type) {
        mpack_write_cstr(writer, "gt");
        mpack_write_i8(writer, *geometry_type);
    }
    mpack_write_cstr(writer, "gp");
    mpack_start_array(writer, properties.size());
    for (auto prop : properties) {
        mpack_write_double(writer, prop);
    }
    mpack_finish_array(writer);
    if (on_click_member) {
        mpack_write_cstr(writer, "L");
        mpack_write_cstr(writer, on_click_member->u8String().c_str());
    }
    if (on_click_field) {
        mpack_write_cstr(writer, "l");
        mpack_write_cstr(writer, on_click_field->u8String().c_str());
    }
    if (!text.empty()) {
        mpack_write_cstr(writer, "x");
        mpack_write_cstr(writer, text.u8String().c_str());
    }
    mpack_complete_map(writer);
}
    Canvas2DComponentData Canvas2DComponentData::parse(const mpack_node_t &writer){
    Canvas2DComponentData data;
    data.type = mpack_node_i8(mpack_node_map_cstr(writer, "t"));
    auto op = mpack_node_map_cstr(writer, "op");
    data.origin_pos[0] = mpack_node_double(mpack_node_array_at(op, 0));
    data.origin_pos[1] = mpack_node_double(mpack_node_array_at(op, 1));
    data.origin_rot = mpack_node_double(mpack_node_map_cstr(writer, "or"));
    data.color = mpack_node_i8(mpack_node_map_cstr(writer, "c"));
    data.fill = mpack_node_i8(mpack_node_map_cstr(writer, "f"));
    data.stroke_width = mpack_node_double(mpack_node_map_cstr(writer, "s"));
    if (mpack_node_map_contains_cstr(writer, "gt")) {
        data.geometry_type = mpack_node_i8(mpack_node_map_cstr(writer, "gt"));
    }
    auto gp = mpack_node_map_cstr(writer, "gp");
    data.properties.resize(mpack_node_array_length(gp));
    for (size_t i = 0; i < data.properties.size(); i++) {
        data.properties[i] = mpack_node_double(mpack_node_array_at(gp, i));
    }
    if (mpack_node_map_contains_cstr(writer, "L")) {
        auto l = mpack_node_map_cstr(writer, "L");
        data.on_click_member = SharedString::fromU8String({mpack_node_str(l), mpack_node_strlen(l)});
    }
}

} // namespace message
WEBCFACE_NS_END

#include <gtest/gtest.h>
#include "webcface/common/internal/message/canvas3d.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/component_internal.h"
#include <webcface/member.h>
#include <webcface/canvas3d.h>
#include <webcface/func.h>
#include <stdexcept>

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class Canvas3DTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<internal::ClientData>(self_name);
        callback_called = 0;
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<internal::ClientData> data_;
    FieldBase fieldBase(const SharedString &member,
                        std::string_view name) const {
        return FieldBase{member, SharedString::fromU8String(name)};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{SharedString::fromU8String(member),
                         SharedString::fromU8String(name)};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString::fromU8String(name)};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, SharedString::fromU8String(member),
                     SharedString::fromU8String(name)};
    }
    template <typename T1, typename T2>
    Canvas3D canvas(const T1 &member, const T2 &name) {
        return Canvas3D{field(member, name)};
    }
    template <typename T1, typename T2>
    RobotModel robot_model(const T1 &member, const T2 &name) {
        return RobotModel{field(member, name)};
    }
    int callback_called;
    template <typename V = Canvas3D>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(Canvas3DTest, field) {
    EXPECT_EQ(canvas("a", "b").member().name(), "a");
    EXPECT_EQ(canvas("a", "b").name(), "b");
    EXPECT_EQ(canvas("a", "b").child("c").name(), "b.c");

    EXPECT_THROW(Canvas3D().tryGet(), std::runtime_error);
}
TEST_F(Canvas3DTest, eventTarget) {
    canvas("a", "b").onChange(callback<Canvas3D>());
    data_->canvas3d_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(Canvas3DTest, set) {
    data_->canvas3d_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Canvas3D)>>(callback());
    using namespace webcface::geometries;
    using namespace webcface::robot_joints;
    robot_model(self_name, "b")
        .set({
            RobotLink{"l0", Geometry{}, ViewColor::black},
            RobotLink{"l1", rotationalJoint("j0", "l0", {0, 0, 0, 0, 0, 0}),
                      Geometry{}, ViewColor::black},
        });

    auto v = canvas(self_name, "b");
    v.add(line({0, 0, 0}, {3, 3, 3})
              .origin({1, 1, 1, 0, 0, 0})
              .color(ViewColor::red));
    v.add(plane({0, 0, 0, 0, 0, 0}, 10, 10)
              .origin({2, 2, 2, 0, 0, 0})
              .color(ViewColor::yellow));
    v.add(robot_model(self_name, "b")
              .origin({3, 3, 3, 0, 0, 0})
              .angle("j0", 123));
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &canvas3d_data_base =
        **data_->canvas3d_store.getRecv(self_name, "b"_ss);
    ASSERT_EQ(canvas3d_data_base.components.size(), 3u);
    ASSERT_EQ(canvas3d_data_base.data_ids.size(), 3u);
    std::vector<std::shared_ptr<message::Canvas3DComponentData>> canvas3d_data;
    canvas3d_data.reserve(canvas3d_data_base.components.size());
    for (const auto &id : canvas3d_data_base.data_ids) {
        canvas3d_data.push_back(
            canvas3d_data_base.components.at(id.u8String()));
    }
    EXPECT_EQ(canvas3d_data[0]->type,
              static_cast<int>(Canvas3DComponentType::geometry));
    EXPECT_EQ(canvas3d_data[0]->origin_pos, (std::array<double, 3>{1, 1, 1}));
    EXPECT_EQ(canvas3d_data[0]->origin_rot, (std::array<double, 3>{0, 0, 0}));
    EXPECT_EQ(canvas3d_data[0]->color, static_cast<int>(ViewColor::red));
    EXPECT_EQ(canvas3d_data[0]->geometry_type,
              static_cast<int>(GeometryType::line));
    EXPECT_EQ(canvas3d_data[0]->geometry_properties,
              (std::vector<double>{0, 0, 0, 3, 3, 3}));
    EXPECT_EQ(canvas3d_data[0]->field_member, std::nullopt);
    EXPECT_EQ(canvas3d_data[0]->field_field, std::nullopt);
    // EXPECT_EQ(canvas3d_data[0].angles().size(), 0); todo:
    // angle取得できないの?

    EXPECT_EQ(canvas3d_data[1]->type,
              static_cast<int>(Canvas3DComponentType::geometry));
    EXPECT_EQ(canvas3d_data[1]->origin_pos, (std::array<double, 3>{2, 2, 2}));
    EXPECT_EQ(canvas3d_data[1]->origin_rot, (std::array<double, 3>{0, 0, 0}));
    EXPECT_EQ(canvas3d_data[1]->color, static_cast<int>(ViewColor::yellow));
    EXPECT_EQ(canvas3d_data[1]->geometry_type,
              static_cast<int>(GeometryType::plane));
    EXPECT_EQ(canvas3d_data[1]->geometry_properties,
              (std::vector<double>{0, 0, 0, 0, 0, 0, 10, 10}));
    EXPECT_EQ(canvas3d_data[1]->field_member, std::nullopt);
    EXPECT_EQ(canvas3d_data[1]->field_field, std::nullopt);
    // EXPECT_EQ(canvas3d_data[1].angles_.size(), 0);

    EXPECT_EQ(canvas3d_data[2]->type,
              static_cast<int>(Canvas3DComponentType::robot_model));
    EXPECT_EQ(canvas3d_data[2]->origin_pos, (std::array<double, 3>{3, 3, 3}));
    EXPECT_EQ(canvas3d_data[2]->origin_rot, (std::array<double, 3>{0, 0, 0}));
    EXPECT_EQ(canvas3d_data[2]->color, static_cast<int>(ViewColor::inherit));
    EXPECT_EQ(canvas3d_data[2]->geometry_type, std::nullopt);
    EXPECT_EQ(canvas3d_data[2]->field_member->u8String(), self_name.decode());
    EXPECT_EQ(canvas3d_data[2]->field_field->u8String(), "b");
    // ASSERT_EQ(canvas3d_data[2].angles_.size(), 1);
    // EXPECT_EQ(canvas3d_data[2].angles_.at(1), 123);

    v.init();
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(
        (*data_->canvas3d_store.getRecv(self_name, "b"_ss))->components.size(),
        0u);

    {
        auto v2 = canvas(self_name, "b");
        v2.add(TemporalCanvas3DComponent{Canvas3DComponentType::geometry});
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ(
        (*data_->canvas3d_store.getRecv(self_name, "b"_ss))->components.size(),
        1u);

    {
        Canvas3D v3;
        {
            Canvas3D v4 = canvas(self_name, "b");
            v4.add(TemporalCanvas3DComponent{Canvas3DComponentType::geometry});
            v3 = v4;
        } // v3にコピーされてるのでまだsyncされない
        EXPECT_EQ(callback_called, 3);
    } // v3のデストラクタでsyncされる
    EXPECT_EQ(callback_called, 4);

    { Canvas3D v5{}; } // エラーやセグフォしない

    Canvas3D v6{};
    v6.add(TemporalCanvas3DComponent{Canvas3DComponentType::geometry});
    EXPECT_THROW(v6.sync(), std::runtime_error);
}
TEST_F(Canvas3DTest, get) {
    auto vd = std::make_shared<message::Canvas3DData>();
    vd->components = {
        {"0", std::make_shared<message::Canvas3DComponentData>()}};
    vd->data_ids = {"0"_ss};
    data_->canvas3d_store.setRecv("a"_ss, "b"_ss, vd);
    EXPECT_EQ(canvas("a", "b").tryGet().value().size(), 1u);
    EXPECT_EQ(canvas("a", "b").get().size(), 1u);
    EXPECT_EQ(canvas("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(canvas("a", "c").get().size(), 0u);
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(canvas(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->canvas3d_store.transferReq().count(self_name), 0u);
    canvas("a", "d").onChange(callback<Canvas3D>());
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}

// todo: hidden, free

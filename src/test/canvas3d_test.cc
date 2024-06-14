#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/canvas3d.h>
#include <webcface/func.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(Encoding::castToU8(std::string_view(str, len)));
}

class Canvas3DTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<Internal::ClientData> data_;
    FieldBase fieldBase(const SharedString &member,
                        std::string_view name) const {
        return FieldBase{member, SharedString(Encoding::castToU8(name))};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{SharedString(Encoding::castToU8(member)),
                         SharedString(Encoding::castToU8(name))};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString(Encoding::castToU8(name))};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, SharedString(Encoding::castToU8(member)),
                     SharedString(Encoding::castToU8(name))};
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
    canvas("a", "b").appendListener(callback<Canvas3D>());
    data_->canvas3d_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(Canvas3DTest, set) {
    (data_->canvas3d_change_event[self_name]["b"_ss] =
         std::make_shared<eventpp::CallbackList<void(Canvas3D)>>())
        ->append(callback());
    using namespace webcface::Geometries;
    using namespace webcface::RobotJoints;
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
    auto &canvas3d_data = **data_->canvas3d_store.getRecv(self_name, "b"_ss);
    ASSERT_EQ(canvas3d_data.size(), 3);
    EXPECT_EQ(canvas3d_data[0].type_, Canvas3DComponentType::geometry);
    EXPECT_EQ(canvas3d_data[0].origin_, Transform(1, 1, 1, 0, 0, 0));
    EXPECT_EQ(canvas3d_data[0].color_, ViewColor::red);
    ASSERT_NE(canvas3d_data[0].geometry_, std::nullopt);
    EXPECT_EQ(canvas3d_data[0].geometry_->type, GeometryType::line);
    EXPECT_EQ(canvas3d_data[0].geometry_->properties,
              (std::vector<double>{0, 0, 0, 3, 3, 3}));
    EXPECT_EQ(canvas3d_data[0].field_base_, std::nullopt);
    EXPECT_EQ(canvas3d_data[0].angles_.size(), 0);

    EXPECT_EQ(canvas3d_data[1].type_, Canvas3DComponentType::geometry);
    EXPECT_EQ(canvas3d_data[1].origin_, Transform(2, 2, 2, 0, 0, 0));
    EXPECT_EQ(canvas3d_data[1].color_, ViewColor::yellow);
    ASSERT_NE(canvas3d_data[1].geometry_, std::nullopt);
    EXPECT_EQ(canvas3d_data[1].geometry_->type, GeometryType::plane);
    EXPECT_EQ(canvas3d_data[1].geometry_->properties,
              (std::vector<double>{0, 0, 0, 0, 0, 0, 10, 10}));
    EXPECT_EQ(canvas3d_data[1].field_base_, std::nullopt);
    EXPECT_EQ(canvas3d_data[1].angles_.size(), 0);

    EXPECT_EQ(canvas3d_data[2].type_, Canvas3DComponentType::robot_model);
    EXPECT_EQ(canvas3d_data[2].origin_, Transform(3, 3, 3, 0, 0, 0));
    EXPECT_EQ(canvas3d_data[2].color_, ViewColor::inherit);
    EXPECT_EQ(canvas3d_data[2].geometry_, std::nullopt);
    ASSERT_NE(canvas3d_data[2].field_base_, std::nullopt);
    EXPECT_EQ(canvas3d_data[2].field_base_->member_, self_name);
    EXPECT_EQ(canvas3d_data[2].field_base_->field_, "b"_ss);
    ASSERT_EQ(canvas3d_data[2].angles_.size(), 1);
    EXPECT_EQ(canvas3d_data[2].angles_.at(1), 123);

    v.init();
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ((*data_->canvas3d_store.getRecv(self_name, "b"_ss))->size(), 0);

    {
        auto v2 = canvas(self_name, "b");
        v2.add(Canvas3DComponent{});
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ((*data_->canvas3d_store.getRecv(self_name, "b"_ss))->size(), 1);

    {
        Canvas3D v3;
        {
            Canvas3D v4 = canvas(self_name, "b");
            v4.add(Canvas3DComponent{});
            v3 = v4;
        } // v3にコピーされてるのでまだsyncされない
        EXPECT_EQ(callback_called, 3);
    } // v3のデストラクタでsyncされる
    EXPECT_EQ(callback_called, 4);

    { Canvas3D v5{}; } // エラーやセグフォしない

    Canvas3D v6{};
    v6.add(Canvas3DComponent{});
    ;
    EXPECT_THROW(v6.sync(), std::runtime_error);
}
TEST_F(Canvas3DTest, get) {
    auto vd = std::make_shared<std::vector<Canvas3DComponentBase>>(1);
    data_->canvas3d_store.setRecv("a"_ss, "b"_ss, vd);
    EXPECT_EQ(canvas("a", "b").tryGet().value().size(), 1);
    EXPECT_EQ(canvas("a", "b").get().size(), 1);
    EXPECT_EQ(canvas("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(canvas("a", "c").get().size(), 0);
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a"_ss).at("b"_ss), 1);
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a"_ss).at("c"_ss), 2);
    EXPECT_EQ(canvas(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->canvas3d_store.transferReq().count(self_name), 0);
    canvas("a", "d").appendListener(callback<Canvas3D>());
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a"_ss).at("d"_ss), 3);
}

// todo: hidden, free

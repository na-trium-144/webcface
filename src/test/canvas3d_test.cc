#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/canvas3d.h>
#include <webcface/func.h>
#include <stdexcept>
#include <chrono>

using namespace WEBCFACE_NS;
class Canvas3DTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<Internal::ClientData> data_;
    Canvas3D canvas(const std::string &member, const std::string &field) {
        return Canvas3D{Field{data_, member, field}};
    }
    RobotModel robot_model(const std::string &member,
                           const std::string &field) {
        return RobotModel{Field{data_, member, field}};
    }
    int callback_called;
    template <typename V = FieldBase>
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
    data_->canvas3d_change_event.dispatch(FieldBase{"a", "b"},
                                          Field{data_, "a", "b"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(Canvas3DTest, set) {
    data_->canvas3d_change_event.appendListener(FieldBase{self_name, "b"},
                                                callback());
    using namespace WEBCFACE_NS::Geometries;
    using namespace WEBCFACE_NS::RobotJoints;
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
    auto &canvas3d_data = **data_->canvas3d_store.getRecv(self_name, "b");
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
    EXPECT_EQ(canvas3d_data[2].field_base_->field_, "b");
    ASSERT_EQ(canvas3d_data[2].angles_.size(), 1);
    EXPECT_EQ(canvas3d_data[2].angles_.at(1), 123);

    v.init();
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ((*data_->canvas3d_store.getRecv(self_name, "b"))->size(), 0);

    {
        auto v2 = canvas(self_name, "b");
        v2.add(Canvas3DComponent{});
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ((*data_->canvas3d_store.getRecv(self_name, "b"))->size(), 1);

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
    data_->canvas3d_store.setRecv("a", "b", vd);
    EXPECT_EQ(canvas("a", "b").tryGet().value().size(), 1);
    EXPECT_EQ(canvas("a", "b").get().size(), 1);
    EXPECT_EQ(canvas("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(canvas("a", "c").get().size(), 0);
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a").at("b"), 1);
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a").at("c"), 2);
    EXPECT_EQ(canvas(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->canvas3d_store.transferReq().count(self_name), 0);
    canvas("a", "d").appendListener(callback<Canvas3D>());
    EXPECT_EQ(data_->canvas3d_store.transferReq().at("a").at("d"), 3);
}

// todo: hidden, free

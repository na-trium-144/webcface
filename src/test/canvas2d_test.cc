#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/canvas2d.h>
#include <webcface/func.h>
#include <stdexcept>
#include <chrono>

using namespace WEBCFACE_NS;
class Canvas2DTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<Internal::ClientData> data_;
    Canvas2D canvas(const std::string &member, const std::string &field) {
        return Canvas2D{Field{data_, member, field}};
    }
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(Canvas2DTest, field) {
    EXPECT_EQ(canvas("a", "b").member().name(), "a");
    EXPECT_EQ(canvas("a", "b").name(), "b");
    EXPECT_EQ(canvas("a", "b").child("c").name(), "b.c");

    EXPECT_THROW(Canvas2D().tryGet(), std::runtime_error);
}
TEST_F(Canvas2DTest, eventTarget) {
    canvas("a", "b").appendListener(callback<Canvas2D>());
    data_->canvas2d_change_event.dispatch(FieldBase{"a", "b"},
                                          Field{data_, "a", "b"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(Canvas2DTest, set) {
    data_->canvas2d_change_event.appendListener(FieldBase{self_name, "b"},
                                                callback());
    using namespace WEBCFACE_NS::Geometries;

    auto v = canvas(self_name, "b").init(100, 150);
    v.add(line({0, 0}, {3, 3}).color(ViewColor::red));
    v.add(plane({0, 0}, 10, 10).color(ViewColor::yellow));
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &canvas2d_data = **data_->canvas2d_store.getRecv(self_name, "b");
    EXPECT_EQ(canvas2d_data.width, 100);
    EXPECT_EQ(canvas2d_data.height, 150);
    ASSERT_EQ(canvas2d_data.components.size(), 2);
    EXPECT_EQ(canvas2d_data.components[0].type_,
              Canvas2DComponentType::geometry);
    EXPECT_EQ(canvas2d_data.components[0].color_, ViewColor::red);
    ASSERT_NE(canvas2d_data.components[0].geometry_, std::nullopt);
    EXPECT_EQ(canvas2d_data.components[0].geometry_->type, GeometryType::line);
    EXPECT_EQ(canvas2d_data.components[0].geometry_->properties,
              (std::vector<double>{0, 0, 0, 3, 3, 0}));

    EXPECT_EQ(canvas2d_data.components[1].type_,
              Canvas2DComponentType::geometry);
    EXPECT_EQ(canvas2d_data.components[1].color_, ViewColor::yellow);
    ASSERT_NE(canvas2d_data.components[1].geometry_, std::nullopt);
    EXPECT_EQ(canvas2d_data.components[1].geometry_->type, GeometryType::plane);
    EXPECT_EQ(canvas2d_data.components[1].geometry_->properties,
              (std::vector<double>{0, 0, 0, 0, 0, 0, 10, 10}));

    v.init(1, 1);
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(
        (*data_->canvas2d_store.getRecv(self_name, "b"))->components.size(), 0);

    {
        auto v2 = canvas(self_name, "b");
        v2.init(1, 1);
        v2.add(Canvas2DComponent{});
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ(
        (*data_->canvas2d_store.getRecv(self_name, "b"))->components.size(), 1);

    {
        Canvas2D v3;
        {
            Canvas2D v4 = canvas(self_name, "b");
            v4.init(1, 1);
            v4.add(Canvas2DComponent{});
            v3 = v4;
        } // v3にコピーされてるのでまだsyncされない
        EXPECT_EQ(callback_called, 3);
    } // v3のデストラクタでsyncされる
    EXPECT_EQ(callback_called, 4);

    { Canvas2D v5{}; } // エラーやセグフォしない

    Canvas2D v6{};
    v6.init(1, 1);
    v6.add(Canvas2DComponent{});
    EXPECT_THROW(v6.sync(), std::runtime_error);

    Canvas2D v7{};
    EXPECT_THROW(v7.add(Canvas2DComponent{}), std::invalid_argument);
}
TEST_F(Canvas2DTest, get) {
    auto vd = std::make_shared<Common::Canvas2DDataBase>();
    vd->components.resize(1);
    data_->canvas2d_store.setRecv("a", "b", vd);
    EXPECT_EQ(canvas("a", "b").tryGet().value().size(), 1);
    EXPECT_EQ(canvas("a", "b").get().size(), 1);
    EXPECT_EQ(canvas("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(canvas("a", "c").get().size(), 0);
    EXPECT_EQ(data_->canvas2d_store.transferReq().at("a").at("b"), 1);
    EXPECT_EQ(data_->canvas2d_store.transferReq().at("a").at("c"), 2);
    EXPECT_EQ(canvas(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->canvas2d_store.transferReq().count(self_name), 0);
    canvas("a", "d").appendListener(callback<Canvas2D>());
    EXPECT_EQ(data_->canvas2d_store.transferReq().at("a").at("d"), 3);
}

// todo: hidden, free

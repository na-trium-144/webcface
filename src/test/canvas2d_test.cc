#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/canvas2d.h>
#include <webcface/func.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;
class Canvas2DTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::u8string self_name = u8"test";
    std::shared_ptr<Internal::ClientData> data_;
    FieldBase fieldBase(std::u8string_view member,
                        std::string_view name) const {
        return FieldBase{member, Encoding::castToU8(name)};
    }
    FieldBase fieldBase(std::string_view member, std::string_view name) const {
        return FieldBase{Encoding::castToU8(member), Encoding::castToU8(name)};
    }
    Field field(std::u8string_view member, std::string_view name = "") const {
        return Field{data_, member, Encoding::castToU8(name)};
    }
    Field field(std::string_view member, std::string_view name = "") const {
        return Field{data_, Encoding::castToU8(member),
                     Encoding::castToU8(name)};
    }
    template <typename T1, typename T2>
    Canvas2D canvas(const T1 &member, const T2 &name) {
        return Canvas2D{field(member, name)};
    }
    template <typename T1, typename T2>
    Func func(const T1 &member, const T2 &name) {
        return Func{field(member, name)};
    }
    template <typename T>
    AnonymousFunc afunc1(const T &func) {
        return AnonymousFunc{field(self_name, ""), func};
    }
    template <typename T>
    AnonymousFunc afunc2(const T &func) {
        return AnonymousFunc{func};
    }
    int callback_called;
    template <typename V = Canvas2D>
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
    data_->canvas2d_change_event[fieldBase("a", "b")]->operator()(
        field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(Canvas2DTest, set) {
    (data_->canvas2d_change_event[fieldBase(self_name, "b")] =
         std::make_shared<eventpp::CallbackList<void(Canvas2D)>>())
        ->append(callback());
    using namespace webcface::Geometries;

    auto v = canvas(self_name, "b").init(100, 150);
    v.add(line({0, 0}, {3, 3})
              .color(ViewColor::red)
              .onClick(func(self_name, "f")));
    v.add(
        plane({0, 0}, 10, 10).color(ViewColor::yellow).onClick(afunc1([] {})));
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &canvas2d_data = **data_->canvas2d_store.getRecv(self_name, u8"b");
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
    ASSERT_NE(canvas2d_data.components[0].on_click_func_, std::nullopt);
    EXPECT_EQ(canvas2d_data.components[0].on_click_func_->member_, self_name);
    EXPECT_EQ(canvas2d_data.components[0].on_click_func_->field_, u8"f");

    EXPECT_EQ(canvas2d_data.components[1].type_,
              Canvas2DComponentType::geometry);
    EXPECT_EQ(canvas2d_data.components[1].color_, ViewColor::yellow);
    ASSERT_NE(canvas2d_data.components[1].geometry_, std::nullopt);
    EXPECT_EQ(canvas2d_data.components[1].geometry_->type, GeometryType::plane);
    EXPECT_EQ(canvas2d_data.components[1].geometry_->properties,
              (std::vector<double>{0, 0, 0, 0, 0, 0, 10, 10}));
    ASSERT_NE(canvas2d_data.components[0].on_click_func_, std::nullopt);
    EXPECT_EQ(canvas2d_data.components[0].on_click_func_->member_, self_name);
    EXPECT_NE(canvas2d_data.components[0].on_click_func_->field_, u8"");

    v.init(1, 1);
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(
        (*data_->canvas2d_store.getRecv(self_name, u8"b"))->components.size(),
        0);

    {
        auto v2 = canvas(self_name, "b");
        v2.init(1, 1);
        v2.add(Canvas2DComponent{});
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ(
        (*data_->canvas2d_store.getRecv(self_name, u8"b"))->components.size(),
        1);

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
    data_->canvas2d_store.setRecv(u8"a", u8"b", vd);
    EXPECT_EQ(canvas("a", "b").tryGet().value().size(), 1);
    EXPECT_EQ(canvas("a", "b").get().size(), 1);
    EXPECT_EQ(canvas("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(canvas("a", "c").get().size(), 0);
    EXPECT_EQ(data_->canvas2d_store.transferReq().at(u8"a").at(u8"b"), 1);
    EXPECT_EQ(data_->canvas2d_store.transferReq().at(u8"a").at(u8"c"), 2);
    EXPECT_EQ(canvas(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->canvas2d_store.transferReq().count(self_name), 0);
    canvas("a", "d").appendListener(callback<Canvas2D>());
    EXPECT_EQ(data_->canvas2d_store.transferReq().at(u8"a").at(u8"d"), 3);
}

// todo: hidden, free

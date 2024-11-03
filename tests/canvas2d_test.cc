#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/canvas2d.h>
#include <webcface/func.h>
#include <stdexcept>

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class Canvas2DTest : public ::testing::Test {
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
    canvas("a", "b").onChange(callback<Canvas2D>());
    data_->canvas2d_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(Canvas2DTest, set) {
    data_->canvas2d_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Canvas2D)>>(callback());
    using namespace webcface::geometries;

    auto v = canvas(self_name, "b").init(100, 150);
    v.add(line({0, 0}, {3, 3})
              .color(ViewColor::red)
              .onClick(func(self_name, "f")));
    v.add(plane(translation(0, 0), 10, 10)
              .color(ViewColor::yellow)
              .onClick(afunc1([] {})));
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &canvas2d_data = **data_->canvas2d_store.getRecv(self_name, "b"_ss);
    EXPECT_EQ(canvas2d_data.width, 100);
    EXPECT_EQ(canvas2d_data.height, 150);
    ASSERT_EQ(canvas2d_data.components.size(), 2u);
    EXPECT_EQ(canvas2d_data.components[0]->type,
              static_cast<int>(Canvas2DComponentType::geometry));
    EXPECT_EQ(canvas2d_data.components[0]->color,
              static_cast<int>(ViewColor::red));
    EXPECT_EQ(canvas2d_data.components[0]->geometry_type,
              static_cast<int>(GeometryType::line));
    EXPECT_EQ(canvas2d_data.components[0]->properties,
              (std::vector<double>{0, 0, 0, 3, 3, 0}));
    EXPECT_EQ(canvas2d_data.components[0]->on_click_member->u8String(),
              self_name.decode());
    EXPECT_EQ(canvas2d_data.components[0]->on_click_field->u8String(), "f");

    EXPECT_EQ(canvas2d_data.components[1]->type,
              static_cast<int>(Canvas2DComponentType::geometry));
    EXPECT_EQ(canvas2d_data.components[1]->color,
              static_cast<int>(ViewColor::yellow));
    EXPECT_EQ(canvas2d_data.components[1]->geometry_type,
              static_cast<int>(GeometryType::plane));
    EXPECT_EQ(canvas2d_data.components[1]->properties,
              (std::vector<double>{0, 0, 0, 0, 0, 0, 10, 10}));
    EXPECT_EQ(canvas2d_data.components[0]->on_click_member->u8String(),
              self_name.decode());
    EXPECT_NE(canvas2d_data.components[0]->on_click_field->u8String(), "");

    v.init(1, 1);
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ(
        (*data_->canvas2d_store.getRecv(self_name, "b"_ss))->components.size(),
        0u);

    {
        auto v2 = canvas(self_name, "b");
        v2.init(1, 1);
        v2.add(TemporalCanvas2DComponent{Canvas2DComponentType::geometry});
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ(
        (*data_->canvas2d_store.getRecv(self_name, "b"_ss))->components.size(),
        1u);

    {
        Canvas2D v3;
        {
            Canvas2D v4 = canvas(self_name, "b");
            v4.init(1, 1);
            v4.add(TemporalCanvas2DComponent{Canvas2DComponentType::geometry});
            v3 = v4;
        } // v3にコピーされてるのでまだsyncされない
        EXPECT_EQ(callback_called, 3);
    } // v3のデストラクタでsyncされる
    EXPECT_EQ(callback_called, 4);

    { Canvas2D v5{}; } // エラーやセグフォしない

    Canvas2D v6{};
    v6.init(1, 1);
    v6.add(TemporalCanvas2DComponent{Canvas2DComponentType::geometry});
    EXPECT_THROW(v6.sync(), std::runtime_error);

    Canvas2D v7{};
    EXPECT_THROW(
        v7.add(TemporalCanvas2DComponent{Canvas2DComponentType::geometry}),
        std::invalid_argument);
}
TEST_F(Canvas2DTest, get) {
    auto vd = std::make_shared<webcface::internal::Canvas2DDataBase>();
    vd->components = {std::make_shared<internal::Canvas2DComponentData>()};
    data_->canvas2d_store.setRecv("a"_ss, "b"_ss, vd);
    EXPECT_EQ(canvas("a", "b").tryGet().value().size(), 1u);
    EXPECT_EQ(canvas("a", "b").get().size(), 1u);
    EXPECT_EQ(canvas("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(canvas("a", "c").get().size(), 0u);
    EXPECT_EQ(data_->canvas2d_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->canvas2d_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(canvas(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->canvas2d_store.transferReq().count(self_name), 0u);
    canvas("a", "d").onChange(callback<Canvas2D>());
    EXPECT_EQ(data_->canvas2d_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}

// todo: hidden, free

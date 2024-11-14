#include <gtest/gtest.h>
#include "webcface/common/internal/message/view.h"
#include "webcface/internal/client_internal.h"
#include "webcface/internal/component_internal.h"
#include <webcface/member.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/text.h>
#include <stdexcept>

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class ViewTest : public ::testing::Test {
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
    Field field(const SharedString &member, const SharedString &name) const {
        return Field{data_, member, name};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString::fromU8String(name)};
    }
    Field field(std::string_view member, std::string_view name) const {
        return Field{data_, SharedString::fromU8String(member),
                     SharedString::fromU8String(name)};
    }
    template <typename T1, typename T2>
    View view(const T1 &member, const T2 &name) {
        return View{field(member, name)};
    }
    template <typename T1, typename T2>
    Text text(const T1 &member, const T2 &name) {
        return Text{field(member, name)};
    }
    template <typename T1, typename T2>
    Func func(const T1 &member, const T2 &name) {
        return Func{field(member, name)};
    }
    int callback_called;
    template <typename V = View>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(ViewTest, field) {
    EXPECT_EQ(view("a", "b").member().name(), "a");
    EXPECT_EQ(view("a", "b").name(), "b");
    EXPECT_EQ(view("a", "b").child("c").name(), "b.c");

    EXPECT_THROW(View().tryGet(), std::runtime_error);
}
TEST_F(ViewTest, eventTarget) {
    view("a", "b").onChange(callback<View>());
    data_->view_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ViewTest, viewSet) {
    data_->view_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(View)>>(callback());
    using namespace webcface::ViewComponents;
    auto v = view(self_name, "b");
    v << "a\n" << 1;
    v << components::text("aaa")
             .textColor(ViewColor::yellow)
             .bgColor(ViewColor::green)
      << newLine();
    v << button("f", func(self_name, "f"));
    v << button("a", []() {});
    v << button("a2", []() {}).id("hoge");
    webcface::InputRef ref1, ref2;
    v << decimalInput("i").bind(ref1).init(123).min(1).max(1000);
    v << selectInput("i2").bind(ref2).option({"a", "b", "c"});
    int called_ref3 = 0;
    v << textInput("i3").onChange([&](const std::string &val) {
        called_ref3++;
        EXPECT_EQ(val, "aaa");
    });
    int manip_called = 0;
    auto manip2 = [&](const webcface::View &v2) {
        manip_called++;
        EXPECT_EQ(v, v2);
    };
    v << manip2;
    EXPECT_EQ(manip_called, 1);
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &view_data_base = **data_->view_store.getRecv(self_name, "b"_ss);
    EXPECT_EQ(view_data_base.components.size(), 11u);
    EXPECT_EQ(view_data_base.data_ids.size(), 11u);
    std::vector<std::shared_ptr<message::ViewComponentData>> view_data;
    view_data.reserve(view_data_base.components.size());
    for (const auto &id : view_data_base.data_ids) {
        view_data.push_back(view_data_base.components.at(id.u8String()));
    }
    EXPECT_EQ(view_data_base.data_ids[0].u8String(), "..0.0");
    EXPECT_EQ(view_data[0]->type, static_cast<int>(ViewComponentType::text));
    EXPECT_EQ(view_data[0]->text.u8String(), "a");
    EXPECT_EQ(view_data_base.data_ids[1].u8String(), "..1.0");
    EXPECT_EQ(view_data[1]->type,
              static_cast<int>(ViewComponentType::new_line));
    EXPECT_EQ(view_data_base.data_ids[2].u8String(), "..0.1");
    EXPECT_EQ(view_data[2]->type, static_cast<int>(ViewComponentType::text));
    EXPECT_EQ(view_data[2]->text.u8String(), "1");

    EXPECT_EQ(view_data_base.data_ids[3].u8String(), "..0.2");
    EXPECT_EQ(view_data[3]->type, static_cast<int>(ViewComponentType::text));
    EXPECT_EQ(view_data[3]->text.u8String(), "aaa");
    EXPECT_EQ(view_data[3]->text_color, static_cast<int>(ViewColor::yellow));
    EXPECT_EQ(view_data[3]->bg_color, static_cast<int>(ViewColor::green));
    EXPECT_EQ(view_data_base.data_ids[4].u8String(), "..1.1");
    EXPECT_EQ(view_data[4]->type,
              static_cast<int>(ViewComponentType::new_line));

    EXPECT_EQ(view_data_base.data_ids[5].u8String(), "..2.0");
    EXPECT_EQ(view_data[5]->type, static_cast<int>(ViewComponentType::button));
    EXPECT_EQ(view_data[5]->text.u8String(), "f");
    EXPECT_EQ(view_data[5]->on_click_member->u8String(), self_name.decode());
    EXPECT_EQ(view_data[5]->on_click_field->u8String(), "f");

    EXPECT_EQ(view_data_base.data_ids[6].u8String(), "..2.1");
    EXPECT_EQ(view_data[6]->type, static_cast<int>(ViewComponentType::button));
    EXPECT_EQ(view_data[6]->text.u8String(), "a");
    EXPECT_EQ(view_data[6]->on_click_member->u8String(), self_name.decode());
    EXPECT_FALSE(view_data[6]->on_click_field->empty());

    EXPECT_EQ(view_data_base.data_ids[7].u8String(), "hoge");
    EXPECT_EQ(view_data[7]->type, static_cast<int>(ViewComponentType::button));
    EXPECT_EQ(view_data[7]->text.u8String(), "a2");
    EXPECT_EQ(view_data[7]->on_click_member->u8String(), self_name.decode());
    EXPECT_FALSE(view_data[7]->on_click_field->empty());

    EXPECT_EQ(view_data[8]->type,
              static_cast<int>(ViewComponentType::decimal_input));
    EXPECT_EQ(view_data[8]->text.u8String(), "i");
    EXPECT_EQ(view_data[8]->on_click_member->u8String(), self_name.decode());
    EXPECT_FALSE(view_data[8]->on_click_field->empty());
    // EXPECT_EQ(view_data[8].text_ref_->member_, self_name.decode());
    // EXPECT_FALSE(view_data[8].text_ref_->field_.empty());
    EXPECT_EQ(static_cast<int>(ref1.get()), 123);
    // EXPECT_EQ(text(self_name, view_data[8].text_ref_->field_).get(), "123");
    // EXPECT_EQ(
    //     static_cast<int>(
    //         text(self_name,
    //         view_data[8].text_ref_->field_).tryGetV().value()),
    //     123);
    func(self_name, *view_data[8]->on_click_field).runAsync(10);
    EXPECT_EQ(static_cast<int>(ref1.get()), 10);
    // EXPECT_EQ(text(self_name, view_data[8].text_ref_->field_).get(), "10");
    // EXPECT_EQ(
    //     static_cast<int>(
    //         text(self_name,
    //         view_data[8].text_ref_->field_).tryGetV().value()),
    //     10);
    EXPECT_EQ(view_data[8]->min_, 1);
    EXPECT_EQ(view_data[8]->max_, 1000);

    EXPECT_EQ(view_data[9]->type,
              static_cast<int>(ViewComponentType::select_input));
    EXPECT_EQ(view_data[9]->text.u8String(), "i2");
    EXPECT_EQ(view_data[9]->on_click_member->u8String(), self_name.decode());
    EXPECT_FALSE(view_data[9]->on_click_field->empty());
    // EXPECT_EQ(view_data[9].text_ref_->member_, self_name.decode());
    // EXPECT_FALSE(view_data[9].text_ref_->field_.empty());
    EXPECT_EQ(view_data[9]->option_.size(), 3u);
    func(self_name, *view_data[9]->on_click_field).runAsync("a");
    EXPECT_EQ(static_cast<std::string>(ref2.get()), "a");
    // EXPECT_EQ(static_cast<std::string>(
    //               text(self_name, view_data[9].text_ref_->field_).get()),
    //           "a");

    EXPECT_EQ(view_data[10]->type,
              static_cast<int>(ViewComponentType::text_input));
    EXPECT_EQ(view_data[10]->text.u8String(), "i3");
    EXPECT_EQ(view_data[10]->on_click_member->u8String(), self_name.decode());
    EXPECT_FALSE(view_data[10]->on_click_field->empty());
    // EXPECT_EQ(view_data[10].text_ref_->member_, self_name.decode());
    // EXPECT_FALSE(view_data[10].text_ref_->field_.empty());
    func(self_name, *view_data[10]->on_click_field).runAsync("aaa");
    EXPECT_EQ(called_ref3, 1);
    // EXPECT_EQ(static_cast<std::string>(
    //               text(self_name, view_data[10].text_ref_->field_).get()),
    //           "aaa");

    v.init();
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ((*data_->view_store.getRecv(self_name, "b"_ss))->data_ids.size(),
              0u);

    {
        auto v2 = view(self_name, "b");
        v2 << "a";
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ((*data_->view_store.getRecv(self_name, "b"_ss))->data_ids.size(),
              1u);

    {
        View v3;
        {
            View v4 = view(self_name, "b");
            v4 << "a";
            v3 = v4;
        } // v3にコピーされてるのでまだsyncされない
        EXPECT_EQ(callback_called, 3);
    } // v3のデストラクタでsyncされる
    EXPECT_EQ(callback_called, 4);

    { View v5{}; } // エラーやセグフォしない

    View v6{};
    v6 << "a";
    EXPECT_THROW(v6.sync(), std::runtime_error);
}
TEST_F(ViewTest, viewGet) {
    std::unordered_map<ViewComponentType, int> idx_next;
    auto vd = std::make_shared<message::ViewData>();
    vd->components = {
        {"a",
         std::static_pointer_cast<message::ViewComponentData>(
             std::shared_ptr(components::text("a").id("a").component_v.lockTmp(
                 data_, "b"_ss, &idx_next)))},
        {"c",
         std::static_pointer_cast<message::ViewComponentData>(
             std::shared_ptr(components::text("a").id("c").component_v.lockTmp(
                 data_, "b"_ss, &idx_next)))},
        {"b", std::static_pointer_cast<message::ViewComponentData>(
                  std::shared_ptr(components::button("a", [] {})
                                      .id("b")
                                      .lockTmp(data_, "b"_ss, &idx_next)))},
    };
    vd->data_ids = {"a"_ss, "b"_ss, "c"_ss};
    data_->view_store.setRecv("a"_ss, "b"_ss, vd);
    EXPECT_EQ(view("a", "b").tryGet().value().size(), 3u);
    EXPECT_EQ(view("a", "b").get().size(), 3u);
    auto components = view("a", "b").get();
    EXPECT_EQ(components.at(0).type(), ViewComponentType::text);
    EXPECT_EQ(components.at(0).text(), "a");
    EXPECT_EQ(components.at(0).id(), "a");
    EXPECT_EQ(components.at(1).type(), ViewComponentType::button);
    EXPECT_EQ(components.at(1).text(), "a");
    EXPECT_EQ(components.at(1).id(), "b");
    ASSERT_TRUE(components.at(1).onClick().has_value());
    EXPECT_EQ(components.at(1).onClick()->member().name(), self_name.decode());
    EXPECT_EQ(components.at(1).onClick()->name(), "..vb/b");
    EXPECT_EQ(components.at(2).type(), ViewComponentType::text);
    EXPECT_EQ(components.at(2).text(), "a");
    EXPECT_EQ(components.at(2).id(), "c");

    EXPECT_EQ(view("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(view("a", "c").get().size(), 0u);

    EXPECT_EQ(data_->view_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->view_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(view(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->view_store.transferReq().count(self_name), 0u);
    view("a", "d").onChange(callback<View>());
    EXPECT_EQ(data_->view_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}

// todo: hidden, free

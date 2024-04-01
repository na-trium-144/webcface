#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/text.h>
#include <stdexcept>
#include <chrono>

using namespace WEBCFACE_NS;
class ViewTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<Internal::ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<Internal::ClientData> data_;
    View view(const std::string &member, const std::string &field) {
        return View{Field{data_, member, field}};
    }
    Text text(const std::string &member, const std::string &field) {
        return Text{Field{data_, member, field}};
    }
    Func func(const std::string &member, const std::string &field) {
        return Func{Field{data_, member, field}};
    }
    template <typename T>
    AnonymousFunc afunc1(const T &func) {
        return AnonymousFunc{Field{data_, self_name, ""}, func};
    }
    int callback_called;
    template <typename V = FieldBase>
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
    view("a", "b").appendListener(callback<View>());
    data_->view_change_event.dispatch(FieldBase{"a", "b"},
                                      Field{data_, "a", "b"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ViewTest, viewSet) {
    data_->view_change_event.appendListener(FieldBase{self_name, "b"},
                                            callback());
    using namespace WEBCFACE_NS::ViewComponents;
    auto v = view(self_name, "b");
    v << "a\n" << 1;
    v << Components::text("aaa")
             .textColor(ViewColor::yellow)
             .bgColor(ViewColor::green)
      << newLine();
    v << button("f", func(self_name, "f"));
    v << button("a", afunc1([]() {}));
    v << button("a2", []() {});
    WEBCFACE_NS::InputRef ref1, ref2;
    v << numInput("i").bind(ref1).init(123).min(1).max(1000);
    v << selectInput("i2").bind(ref2).option({"a", "b", "c"});
    int called_ref3 = 0;
    v << textInput("i3").onChange([&](const std::string &val) {
        called_ref3++;
        EXPECT_EQ(val, "aaa");
    });
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &view_data = **data_->view_store.getRecv(self_name, "b");
    EXPECT_EQ(view_data.size(), 11);
    EXPECT_EQ(view_data[0].type_, ViewComponentType::text);
    EXPECT_EQ(view_data[0].text_, "a");
    EXPECT_EQ(view_data[1].type_, ViewComponentType::new_line);
    EXPECT_EQ(view_data[2].type_, ViewComponentType::text);
    EXPECT_EQ(view_data[2].text_, "1");

    EXPECT_EQ(view_data[3].type_, ViewComponentType::text);
    EXPECT_EQ(view_data[3].text_, "aaa");
    EXPECT_EQ(view_data[3].text_color_, ViewColor::yellow);
    EXPECT_EQ(view_data[3].bg_color_, ViewColor::green);
    EXPECT_EQ(view_data[4].type_, ViewComponentType::new_line);

    EXPECT_EQ(view_data[5].type_, ViewComponentType::button);
    EXPECT_EQ(view_data[5].text_, "f");
    EXPECT_EQ(view_data[5].on_click_func_->member_, self_name);
    EXPECT_EQ(view_data[5].on_click_func_->field_, "f");

    EXPECT_EQ(view_data[6].type_, ViewComponentType::button);
    EXPECT_EQ(view_data[6].text_, "a");
    EXPECT_EQ(view_data[6].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[6].on_click_func_->field_.empty());

    EXPECT_EQ(view_data[7].type_, ViewComponentType::button);
    EXPECT_EQ(view_data[7].text_, "a2");
    EXPECT_EQ(view_data[7].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[7].on_click_func_->field_.empty());

    EXPECT_EQ(view_data[8].type_, ViewComponentType::num_input);
    EXPECT_EQ(view_data[8].text_, "i");
    EXPECT_EQ(view_data[8].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[8].on_click_func_->field_.empty());
    EXPECT_EQ(view_data[8].text_ref_->member_, self_name);
    EXPECT_FALSE(view_data[8].text_ref_->field_.empty());
    EXPECT_EQ(static_cast<int>(ref1.get()), 123);
    EXPECT_EQ(
        static_cast<int>(text(self_name, view_data[8].text_ref_->field_).get()),
        123);
    func(self_name, view_data[8].on_click_func_->field_).run(10);
    EXPECT_EQ(static_cast<int>(ref1.get()), 10);
    EXPECT_EQ(
        static_cast<int>(text(self_name, view_data[8].text_ref_->field_).get()),
        10);
    EXPECT_EQ(view_data[8].min_, 1);
    EXPECT_EQ(view_data[8].max_, 1000);

    EXPECT_EQ(view_data[9].type_, ViewComponentType::select_input);
    EXPECT_EQ(view_data[9].text_, "i2");
    EXPECT_EQ(view_data[9].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[9].on_click_func_->field_.empty());
    EXPECT_EQ(view_data[9].text_ref_->member_, self_name);
    EXPECT_FALSE(view_data[9].text_ref_->field_.empty());
    EXPECT_EQ(view_data[9].option_.size(), 3);
    func(self_name, view_data[9].on_click_func_->field_).run("a");
    EXPECT_EQ(static_cast<std::string>(ref2.get()), "a");
    EXPECT_EQ(static_cast<std::string>(
                  text(self_name, view_data[9].text_ref_->field_).get()),
              "a");

    EXPECT_EQ(view_data[10].type_, ViewComponentType::text_input);
    EXPECT_EQ(view_data[10].text_, "i3");
    EXPECT_EQ(view_data[10].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[10].on_click_func_->field_.empty());
    EXPECT_EQ(view_data[10].text_ref_->member_, self_name);
    EXPECT_FALSE(view_data[10].text_ref_->field_.empty());
    func(self_name, view_data[10].on_click_func_->field_).run("aaa");
    EXPECT_EQ(called_ref3, 1);
    EXPECT_EQ(static_cast<std::string>(
                  text(self_name, view_data[10].text_ref_->field_).get()),
              "aaa");

    v.init();
    v.sync();
    EXPECT_EQ(callback_called, 2);
    EXPECT_EQ((*data_->view_store.getRecv(self_name, "b"))->size(), 0);

    {
        auto v2 = view(self_name, "b");
        v2 << "a";
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ((*data_->view_store.getRecv(self_name, "b"))->size(), 1);

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
    auto vd = std::make_shared<std::vector<ViewComponentBase>>(
        std::vector<ViewComponentBase>{
            Components::text("a").toV().lockTmp(data_, "", nullptr, nullptr)});
    data_->view_store.setRecv("a", "b", vd);
    EXPECT_EQ(view("a", "b").tryGet().value().size(), 1);
    EXPECT_EQ(view("a", "b").get().size(), 1);
    EXPECT_EQ(view("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(view("a", "c").get().size(), 0);
    EXPECT_EQ(data_->view_store.transferReq().at("a").at("b"), 1);
    EXPECT_EQ(data_->view_store.transferReq().at("a").at("c"), 2);
    EXPECT_EQ(view(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->view_store.transferReq().count(self_name), 0);
    view("a", "d").appendListener(callback<View>());
    EXPECT_EQ(data_->view_store.transferReq().at("a").at("d"), 3);
}

// todo: hidden, free

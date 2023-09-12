#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <stdexcept>
#include <chrono>

using namespace WebCFace;
class ViewTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<ClientData> data_;
    View view(const std::string &member, const std::string &field) {
        return View{Field{data_, member, field}};
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
    using namespace WebCFace::ViewComponents;
    auto v = view(self_name, "b");
    v << "a\n" << 1;
    v << text("aaa").textColor(ViewColor::yellow).bgColor(ViewColor::green)
      << newLine();
    v << button("f", func(self_name, "f"));
    v << button("a", afunc1([]() {}));
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &view_data = **data_->view_store.getRecv(self_name, "b");
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
}
TEST_F(ViewTest, viewGet) {
    auto vd = std::make_shared<std::vector<ViewComponentBase>>(
        std::vector<ViewComponentBase>{text("a")});
    data_->view_store.setRecv("a", "b", vd);
    EXPECT_EQ(view("a", "b").tryGet().value().size(), 1);
    EXPECT_EQ(view("a", "b").get().size(), 1);
    EXPECT_EQ(view("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(view("a", "c").get().size(), 0);
    EXPECT_EQ(data_->view_store.transferReq(true).at("a").at("b"), 1);
    EXPECT_EQ(data_->view_store.transferReq(true).at("a").at("c"), 2);
    EXPECT_EQ(view(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->view_store.transferReq(true).count(self_name), 0);
    view("a", "d").appendListener(callback<View>());
    EXPECT_EQ(data_->view_store.transferReq(true).at("a").at("d"), 3);
}

TEST_F(ViewTest, time) {
    auto t = std::chrono::system_clock::now();
    data_->sync_time_store.setRecv("a", t);
    EXPECT_EQ(view("a", "b").time(), t);
}
// todo: hidden, free

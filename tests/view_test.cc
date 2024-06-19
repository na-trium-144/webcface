#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/view.h>
#include <webcface/func.h>
#include <webcface/text.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(Encoding::castToU8(std::string_view(str, len)));
}

class ViewTest : public ::testing::Test {
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
    Field field(const SharedString &member, const SharedString &name) const {
        return Field{data_, member, name};
    }
    Field field(const SharedString &member, std::string_view name = "") const {
        return Field{data_, member, SharedString(Encoding::castToU8(name))};
    }
    Field field(std::string_view member, std::string_view name) const {
        return Field{data_, SharedString(Encoding::castToU8(member)),
                     SharedString(Encoding::castToU8(name))};
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
    template <typename T>
    AnonymousFunc afunc1(const T &func) {
        return AnonymousFunc{field(self_name, ""), func};
    }
    template <typename T>
    AnonymousFunc afunc2(const T &func) {
        return AnonymousFunc{func};
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
    view("a", "b").appendListener(callback<View>());
    data_->view_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(ViewTest, viewSet) {
    (data_->view_change_event[self_name]["b"_ss] =
         std::make_shared<eventpp::CallbackList<void(View)>>())
        ->append(callback());
    using namespace webcface::ViewComponents;
    auto v = view(self_name, "b");
    v << "a\n" << 1;
    v << Components::text("aaa")
             .textColor(ViewColor::yellow)
             .bgColor(ViewColor::green)
      << newLine();
    v << button("f", func(self_name, "f"));
    v << button("a", afunc1([]() {}));
    v << button("a2", []() {});
    webcface::InputRef ref1, ref2;
    v << decimalInput("i").bind(ref1).init(123).min(1).max(1000);
    v << selectInput("i2").bind(ref2).option({"a", "b", "c"});
    int called_ref3 = 0;
    v << textInput("i3").onChange([&](const std::string &val) {
        called_ref3++;
        EXPECT_EQ(val, "aaa");
    });
    int manip_called = 0;
    auto manip = [&](webcface::View &v2) {
        manip_called++;
        EXPECT_EQ(&v, &v2);
    };
    auto manip2 = [&](webcface::View v2) {
        manip_called++;
        EXPECT_EQ(v, v2);
    };
    v << manip << manip2;
    EXPECT_EQ(manip_called, 2);
    v.sync();
    EXPECT_EQ(callback_called, 1);
    auto &view_data = **data_->view_store.getRecv(self_name, "b"_ss);
    EXPECT_EQ(view_data.size(), 11);
    EXPECT_EQ(view_data[0].type_, ViewComponentType::text);
    EXPECT_EQ(view_data[0].text_, "a"_ss);
    EXPECT_EQ(view_data[1].type_, ViewComponentType::new_line);
    EXPECT_EQ(view_data[2].type_, ViewComponentType::text);
    EXPECT_EQ(view_data[2].text_, "1"_ss);

    EXPECT_EQ(view_data[3].type_, ViewComponentType::text);
    EXPECT_EQ(view_data[3].text_, "aaa"_ss);
    EXPECT_EQ(view_data[3].text_color_, ViewColor::yellow);
    EXPECT_EQ(view_data[3].bg_color_, ViewColor::green);
    EXPECT_EQ(view_data[4].type_, ViewComponentType::new_line);

    EXPECT_EQ(view_data[5].type_, ViewComponentType::button);
    EXPECT_EQ(view_data[5].text_, "f"_ss);
    EXPECT_EQ(view_data[5].on_click_func_->member_, self_name);
    EXPECT_EQ(view_data[5].on_click_func_->field_, "f"_ss);

    EXPECT_EQ(view_data[6].type_, ViewComponentType::button);
    EXPECT_EQ(view_data[6].text_, "a"_ss);
    EXPECT_EQ(view_data[6].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[6].on_click_func_->field_.empty());

    EXPECT_EQ(view_data[7].type_, ViewComponentType::button);
    EXPECT_EQ(view_data[7].text_, "a2"_ss);
    EXPECT_EQ(view_data[7].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[7].on_click_func_->field_.empty());

    EXPECT_EQ(view_data[8].type_, ViewComponentType::decimal_input);
    EXPECT_EQ(view_data[8].text_, "i"_ss);
    EXPECT_EQ(view_data[8].on_click_func_->member_, self_name);
    EXPECT_FALSE(view_data[8].on_click_func_->field_.empty());
    EXPECT_EQ(view_data[8].text_ref_->member_, self_name);
    EXPECT_FALSE(view_data[8].text_ref_->field_.empty());
    EXPECT_EQ(static_cast<int>(ref1.get()), 123);
    EXPECT_EQ(text(self_name, view_data[8].text_ref_->field_).get(), "123");
    EXPECT_EQ(
        static_cast<int>(
            text(self_name, view_data[8].text_ref_->field_).tryGetV().value()),
        123);
    func(self_name, view_data[8].on_click_func_->field_).run(10);
    EXPECT_EQ(static_cast<int>(ref1.get()), 10);
    EXPECT_EQ(text(self_name, view_data[8].text_ref_->field_).get(), "10");
    EXPECT_EQ(
        static_cast<int>(
            text(self_name, view_data[8].text_ref_->field_).tryGetV().value()),
        10);
    EXPECT_EQ(view_data[8].min_, 1);
    EXPECT_EQ(view_data[8].max_, 1000);

    EXPECT_EQ(view_data[9].type_, ViewComponentType::select_input);
    EXPECT_EQ(view_data[9].text_, "i2"_ss);
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
    EXPECT_EQ(view_data[10].text_, "i3"_ss);
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
    EXPECT_EQ((*data_->view_store.getRecv(self_name, "b"_ss))->size(), 0);

    {
        auto v2 = view(self_name, "b");
        v2 << "a";
    }
    EXPECT_EQ(callback_called, 3);
    EXPECT_EQ((*data_->view_store.getRecv(self_name, "b"_ss))->size(), 1);

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
    std::unordered_map<int, int> idx_next;
    auto vd = std::make_shared<std::vector<ViewComponentBase>>(
        std::vector<ViewComponentBase>{
            Components::text("a").toV().lockTmp(data_, "b"_ss, &idx_next),
            Components::button("a", [] {}).lockTmp(data_, "b"_ss, &idx_next),
            Components::text("a").toV().lockTmp(data_, "b"_ss, &idx_next),
        });
    data_->view_store.setRecv("a"_ss, "b"_ss, vd);
    EXPECT_EQ(view("a", "b").tryGet().value().size(), 3);
    EXPECT_EQ(view("a", "b").get().size(), 3);
    auto components = view("a", "b").get();
    EXPECT_EQ(components.at(0).type(), ViewComponentType::text);
    EXPECT_EQ(components.at(0).text(), "a");
    EXPECT_EQ(components.at(0).id(), "..0.0"); // type0, idx0
    EXPECT_EQ(components.at(1).type(), ViewComponentType::button);
    EXPECT_EQ(components.at(1).text(), "a");
    EXPECT_EQ(components.at(1).id(), "..2.0"); // type2, idx0
    ASSERT_TRUE(components.at(1).onClick().has_value());
    EXPECT_EQ(components.at(1).onClick()->member().name(), self_name.decode());
    EXPECT_EQ(components.at(1).onClick()->name(), "..vb/..2.0");
    EXPECT_EQ(components.at(2).type(), ViewComponentType::text);
    EXPECT_EQ(components.at(2).text(), "a");
    EXPECT_EQ(components.at(2).id(), "..0.1"); // type0, idx1

    EXPECT_EQ(view("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(view("a", "c").get().size(), 0);

    EXPECT_EQ(data_->view_store.transferReq().at("a"_ss).at("b"_ss), 1);
    EXPECT_EQ(data_->view_store.transferReq().at("a"_ss).at("c"_ss), 2);
    EXPECT_EQ(view(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->view_store.transferReq().count(self_name), 0);
    view("a", "d").appendListener(callback<View>());
    EXPECT_EQ(data_->view_store.transferReq().at("a"_ss).at("d"_ss), 3);
}

// todo: hidden, free

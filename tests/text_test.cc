#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include "webcface/internal/log_history.h"
#include "webcface/field.h"
#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/text.h>
#include <webcface/log.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;
static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class TextTest : public ::testing::Test {
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
    Text text(const T1 &member, const T2 &name) {
        return Text{field(member, name)};
    }
    template <typename T1, typename T2>
    Variant variant(const T1 &member, const T2 &name) {
        return Variant{field(member, name)};
    }
    int callback_called;
    template <typename V>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
    auto callbackVoid() {
        return [&]() { ++callback_called; };
    }
};

TEST_F(TextTest, field) {
    EXPECT_EQ(text("a", "b").member().name(), "a");
    EXPECT_EQ(text("a", "b").name(), "b");
    EXPECT_EQ(text("a", "b").nameW(), L"b");
    EXPECT_EQ(text("a", "b").child("c").name(), "b.c");
    EXPECT_EQ(text("a", "b").child(L"c").name(), "b.c");

    EXPECT_THROW(Text().tryGet(), std::runtime_error);
}
TEST_F(TextTest, eventTarget) {
    text("a", "b").onChange(callback<Text>());
    data_->text_change_event["a"_ss]["b"_ss]->operator()(field("a", "b"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}
TEST_F(TextTest, textSet) {
    data_->text_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Variant)>>(callback<Variant>());
    text(self_name, "b").set("c");
    EXPECT_EQ(
        static_cast<std::string>(*data_->text_store.getRecv(self_name, "b"_ss)),
        "c");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set("c"), std::invalid_argument);
}
TEST_F(TextTest, textSetW) {
    data_->text_change_event[self_name]["b"_ss] =
        std::make_shared<std::function<void(Variant)>>(callback<Variant>());
    text(self_name, "b").set(L"c");
    EXPECT_EQ(
        static_cast<std::string>(*data_->text_store.getRecv(self_name, "b"_ss)),
        "c");
    EXPECT_EQ(callback_called, 1);
    EXPECT_THROW(text("a", "b").set(L"c"), std::invalid_argument);
}
// TEST_F(TextTest, variantSet) {
//     data_->text_change_event[self_name]["b"_ss] =
//         std::make_shared<std::function<void(Variant)>>(callback<Variant>());
//     variant(self_name, "b").set(ValAdaptor(123));
//     EXPECT_EQ(static_cast<std::string>(
//                   *data_->text_store.getRecv(self_name, "b"_ss)),
//               "123");
//     EXPECT_EQ(callback_called, 1);
//     EXPECT_THROW(variant("a", "b").set(ValAdaptor(123)),
//     std::invalid_argument);
// }

TEST_F(TextTest, textGet) {
    data_->text_store.setRecv("a"_ss, "b"_ss,
                              std::make_shared<ValAdaptor>("hoge"));
    ASSERT_NE(text("a", "b").tryGet(), std::nullopt);
    EXPECT_EQ(text("a", "b").tryGet().value(), "hoge");
    EXPECT_EQ(text("a", "b").tryGetW().value(), L"hoge");
    EXPECT_EQ(variant("a", "b").tryGet().value(), ValAdaptor("hoge"));
    EXPECT_EQ(text("a", "b").get(), "hoge");
    EXPECT_EQ(text("a", "b").getW(), L"hoge");
    EXPECT_EQ(variant("a", "b").get().asStringRef(), "hoge");
    EXPECT_EQ(text("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(text("a", "c").tryGetW(), std::nullopt);
    EXPECT_EQ(variant("a", "c").tryGet(), std::nullopt);
    EXPECT_EQ(text("a", "c").get(), "");
    EXPECT_EQ(text("a", "c").getW(), L"");
    EXPECT_TRUE(variant("a", "c").get().empty());
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("b"_ss), 1u);
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("c"_ss), 2u);
    EXPECT_EQ(text(self_name, "b").tryGet(), std::nullopt);
    EXPECT_EQ(data_->text_store.transferReq().count(self_name), 0u);
    text("a", "d").onChange(callback<Text>());
    EXPECT_EQ(data_->text_store.transferReq().at("a"_ss).at("d"_ss), 3u);
}

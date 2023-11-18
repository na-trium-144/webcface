#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/log.h>
#include <webcface/text.h>
#include <webcface/func.h>
#include <webcface/view.h>
#include <stdexcept>
#include <chrono>

using namespace WebCFace;
class MemberTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<ClientData>(self_name);
        callback_called = 0;
    }
    std::string self_name = "test";
    std::shared_ptr<ClientData> data_;
    Member member(const std::string &member) {
        return Member{Field{data_, member}};
    }
    int callback_called;
    template <typename V = FieldBase>
    auto callback() {
        return [&](const V &) { ++callback_called; };
    }
};

TEST_F(MemberTest, field) {
    EXPECT_EQ(member("a").name(), "a");
    EXPECT_EQ(member("a").value("b").member().name(), "a");
    EXPECT_EQ(member("a").value("b").name(), "b");
    EXPECT_EQ(member("a").text("b").member().name(), "a");
    EXPECT_EQ(member("a").text("b").name(), "b");
    EXPECT_EQ(member("a").func("b").member().name(), "a");
    EXPECT_EQ(member("a").func("b").name(), "b");
    EXPECT_EQ(member("a").view("b").member().name(), "a");
    EXPECT_EQ(member("a").view("b").name(), "b");
    EXPECT_EQ(member("a").log().member().name(), "a");
}

TEST_F(MemberTest, getEntry) {
    data_->value_store.setEntry("a");
    data_->value_store.setEntry("a", "a");
    EXPECT_EQ(member("a").values().size(), 1);
    EXPECT_EQ(member("a").values()[0].name(), "a");
    data_->text_store.setEntry("a");
    data_->text_store.setEntry("a", "a");
    EXPECT_EQ(member("a").texts().size(), 1);
    EXPECT_EQ(member("a").texts()[0].name(), "a");
    data_->func_store.setEntry("a");
    data_->func_store.setEntry("a", "a");
    EXPECT_EQ(member("a").funcs().size(), 1);
    EXPECT_EQ(member("a").funcs()[0].name(), "a");
    data_->view_store.setEntry("a");
    data_->view_store.setEntry("a", "a");
    EXPECT_EQ(member("a").views().size(), 1);
    EXPECT_EQ(member("a").views()[0].name(), "a");
}
TEST_F(MemberTest, eventTarget) {
    member("a").onValueEntry().appendListener(callback<Value>());
    data_->value_entry_event.dispatch("a", Field{data_, "a", "a"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onTextEntry().appendListener(callback<Text>());
    data_->text_entry_event.dispatch("a", Field{data_, "a", "a"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onFuncEntry().appendListener(callback<Func>());
    data_->func_entry_event.dispatch("a", Field{data_, "a", "a"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onViewEntry().appendListener(callback<View>());
    data_->view_entry_event.dispatch("a", Field{data_, "a", "a"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onSync().appendListener(callback<Member>());
    data_->sync_event.dispatch("a", Field{data_, "a"});
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
}

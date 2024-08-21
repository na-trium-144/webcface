#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/log.h>
#include <webcface/text.h>
#include <webcface/func.h>
#include <webcface/view.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

class MemberTest : public ::testing::Test {
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
    template <typename T1>
    Member member(const T1 &member) {
        return Member{field(member)};
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
    data_->value_store.setEntry("a"_ss, "a"_ss);
    EXPECT_EQ(member("a").valueEntries().size(), 1u);
    EXPECT_EQ(member("a").valueEntries()[0].name(), "a");
    data_->text_store.setEntry("a"_ss, "a"_ss);
    EXPECT_EQ(member("a").textEntries().size(), 1u);
    EXPECT_EQ(member("a").textEntries()[0].name(), "a");
    data_->func_store.setEntry("a"_ss, "a"_ss);
    EXPECT_EQ(member("a").funcEntries().size(), 1u);
    EXPECT_EQ(member("a").funcEntries()[0].name(), "a");
    data_->view_store.setEntry("a"_ss, "a"_ss);
    EXPECT_EQ(member("a").viewEntries().size(), 1u);
    EXPECT_EQ(member("a").viewEntries()[0].name(), "a");
}
TEST_F(MemberTest, eventTarget) {
    member("a").onValueEntry(callback<Value>());
    data_->value_entry_event["a"_ss]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onTextEntry(callback<Text>());
    data_->text_entry_event["a"_ss]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onFuncEntry(callback<Func>());
    data_->func_entry_event["a"_ss]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onViewEntry(callback<View>());
    data_->view_entry_event["a"_ss]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onSync(callback<Member>());
    data_->sync_event["a"_ss]->operator()(field("a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;

    member("a").onPing(callback<Member>());
    data_->ping_event["a"_ss]->operator()(field("a"));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->ping_status_req);
    callback_called = 0;
}
TEST_F(MemberTest, libVersion) {
    data_->member_ids["a"_ss] = 1;
    data_->member_lib_name[1] = "aaa";
    data_->member_lib_ver[1] = "bbb";
    data_->member_addr[1] = "ccc";
    EXPECT_EQ(member("a").libName(), "aaa");
    EXPECT_EQ(member("a").libVersion(), "bbb");
    EXPECT_EQ(member("a").remoteAddr(), "ccc");
}
TEST_F(MemberTest, PingStatus) {
    data_->member_ids["a"_ss] = 1;
    data_->ping_status =
        std::make_shared<std::unordered_map<unsigned int, int>>(
            std::unordered_map<unsigned int, int>{{1, 10}});
    EXPECT_EQ(member("a").pingStatus(), 10);
    EXPECT_TRUE(data_->ping_status_req);
}
TEST_F(MemberTest, syncTime) {
    auto t = std::chrono::system_clock::now();
    data_->sync_time_store.setRecv("a"_ss, t);
    EXPECT_EQ(member("a").syncTime(), t);
}

#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <webcface/value.h>
#include <webcface/log.h>
#include <webcface/text.h>
#include <webcface/func.h>
#include <webcface/view.h>
#include <stdexcept>
#include <chrono>

using namespace webcface;
class MemberTest : public ::testing::Test {
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
    data_->value_store.setEntry(u8"a", u8"a");
    EXPECT_EQ(member("a").valueEntries().size(), 1);
    EXPECT_EQ(member("a").valueEntries()[0].name(), "a");
    data_->text_store.setEntry(u8"a", u8"a");
    EXPECT_EQ(member("a").textEntries().size(), 1);
    EXPECT_EQ(member("a").textEntries()[0].name(), "a");
    data_->func_store.setEntry(u8"a", u8"a");
    EXPECT_EQ(member("a").funcEntries().size(), 1);
    EXPECT_EQ(member("a").funcEntries()[0].name(), "a");
    data_->view_store.setEntry(u8"a", u8"a");
    EXPECT_EQ(member("a").viewEntries().size(), 1);
    EXPECT_EQ(member("a").viewEntries()[0].name(), "a");
}
TEST_F(MemberTest, eventTarget) {
    member("a").onValueEntry().appendListener(callback<Value>());
    data_->value_entry_event[u8"a"]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onTextEntry().appendListener(callback<Text>());
    data_->text_entry_event[u8"a"]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onFuncEntry().appendListener(callback<Func>());
    data_->func_entry_event[u8"a"]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onViewEntry().appendListener(callback<View>());
    data_->view_entry_event[u8"a"]->operator()(field("a", "a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;
    member("a").onSync().appendListener(callback<Member>());
    data_->sync_event[u8"a"]->operator()(field("a"));
    EXPECT_EQ(callback_called, 1);
    callback_called = 0;

    member("a").onPing().appendListener(callback<Member>());
    data_->ping_event[u8"a"]->operator()(field("a"));
    EXPECT_EQ(callback_called, 1);
    EXPECT_TRUE(data_->ping_status_req);
    callback_called = 0;
}
TEST_F(MemberTest, libVersion) {
    data_->member_ids[u8"a"] = 1;
    data_->member_lib_name[1] = "aaa";
    data_->member_lib_ver[1] = "bbb";
    data_->member_addr[1] = "ccc";
    EXPECT_EQ(member("a").libName(), "aaa");
    EXPECT_EQ(member("a").libVersion(), "bbb");
    EXPECT_EQ(member("a").remoteAddr(), "ccc");
}
TEST_F(MemberTest, PingStatus) {
    data_->member_ids[u8"a"] = 1;
    data_->ping_status =
        std::make_shared<std::unordered_map<unsigned int, int>>(
            std::unordered_map<unsigned int, int>{{1, 10}});
    EXPECT_EQ(member("a").pingStatus(), 10);
    EXPECT_TRUE(data_->ping_status_req);
}
TEST_F(MemberTest, syncTime) {
    auto t = std::chrono::system_clock::now();
    data_->sync_time_store.setRecv(u8"a", t);
    EXPECT_EQ(member("a").syncTime(), t);
}

#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <string>

using namespace webcface;
using namespace webcface::Internal;

class SyncDataStore2Test : public ::testing::Test {
  protected:
    std::string self_name = "test";
    SyncDataStore2<std::string> s2{"test"};
};
TEST_F(SyncDataStore2Test, self) {
    EXPECT_TRUE(s2.isSelf(self_name));
    EXPECT_FALSE(s2.isSelf("hoge"));
    EXPECT_FALSE(s2.isSelf(""));
}
TEST_F(SyncDataStore2Test, setSend) {
    s2.setSend("a", "b");
    EXPECT_EQ(s2.getRecv(self_name, "a"), "b");
    auto send = s2.transferSend(false);
    EXPECT_EQ(send.at("a"), "b");
    EXPECT_EQ(send.size(), 1);
    EXPECT_EQ(s2.transferSend(false).size(), 0);
    EXPECT_EQ(s2.transferSend(true).size(), 1);
}
TEST_F(SyncDataStore2Test, setRecv) {
    s2.setRecv("a", "b", "c");
    EXPECT_EQ(s2.getRecv("a", "b"), "c");
}
TEST_F(SyncDataStore2Test, getRecv) {
    auto recv_empty = s2.getRecv(self_name, "b");
    EXPECT_EQ(recv_empty, std::nullopt);
    EXPECT_EQ(s2.getReq(1, "").first, "");
    EXPECT_EQ(s2.getReq(1, "").second, "");
    EXPECT_EQ(s2.transferReq(false).size(), 0);

    recv_empty = s2.getRecv("a", "b");
    EXPECT_EQ(recv_empty, std::nullopt);
    EXPECT_EQ(s2.getReq(1, "").first, "a");
    EXPECT_EQ(s2.getReq(1, "").second, "b");
    auto req = s2.transferReq(false);
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at("a").at("b"), 1);

    EXPECT_EQ(s2.transferReq(false).size(), 0);
    EXPECT_EQ(s2.transferReq(true).size(), 1);
}
TEST_F(SyncDataStore2Test, getRecvRecurse) {
    auto recv_empty = s2.getRecvRecurse(self_name, "b");
    EXPECT_EQ(recv_empty, std::nullopt);
    EXPECT_EQ(s2.getReq(1, "").first, "");
    EXPECT_EQ(s2.getReq(1, "").second, "");
    EXPECT_EQ(s2.transferReq(false).size(), 0);

    recv_empty = s2.getRecvRecurse("a", "b");
    EXPECT_EQ(recv_empty, std::nullopt);
    EXPECT_EQ(s2.getReq(1, "").first, "a");
    EXPECT_EQ(s2.getReq(1, "").second, "b");
    auto req = s2.transferReq(false);
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at("a").at("b"), 1);

    s2.setRecv("a", "b.a", "a");
    s2.setRecv("a", "b.b", "b");
    recv_empty = s2.getRecvRecurse("a", "b");
    EXPECT_EQ(recv_empty.value()["a"].get(), "a");
    EXPECT_EQ(recv_empty.value()["b"].get(), "b");
    EXPECT_EQ(s2.getReq(1, "a").second, "b.a");
    // unordered_mapなので順番は不明
    EXPECT_TRUE(s2.getReq(2, "").second.starts_with("b."));
    EXPECT_TRUE(s2.getReq(3, "").second.starts_with("b."));
}
TEST_F(SyncDataStore2Test, unsetRecv) {
    s2.setRecv("a", "b", "c");
    s2.getRecv("d", "e");
    s2.unsetRecv("a", "b");
    s2.unsetRecv("d", "e");
    EXPECT_EQ(s2.getReq(1, "").first, "");
    EXPECT_EQ(s2.getReq(1, "").second, "");
    EXPECT_EQ(s2.getRecv("a", "b"), std::nullopt);
}
TEST_F(SyncDataStore2Test, setEntry) {
    s2.setEntry("a");
    EXPECT_EQ(s2.getMembers().size(), 1);
    EXPECT_EQ(s2.getMembers().at(0), "a");
    s2.setEntry("a", "b");
    EXPECT_EQ(s2.getEntry("a").size(), 1);
    EXPECT_EQ(s2.getEntry("a").at(0), "b");
}

class SyncDataStore1Test : public ::testing::Test {
  protected:
    std::string self_name = "test";
    SyncDataStore1<std::string> s1{"test"};
};
TEST_F(SyncDataStore1Test, self) {
    EXPECT_TRUE(s1.isSelf(self_name));
    EXPECT_FALSE(s1.isSelf("hoge"));
    EXPECT_FALSE(s1.isSelf(""));
}
TEST_F(SyncDataStore1Test, setRecv) {
    s1.setRecv("a", "c");
    EXPECT_EQ(s1.getRecv("a"), "c");
}
TEST_F(SyncDataStore1Test, getRecv) {
    auto recv_empty = s1.getRecv(self_name);
    EXPECT_EQ(recv_empty, std::nullopt);
    EXPECT_EQ(s1.transferReq(false).size(), 0);

    recv_empty = s1.getRecv("a");
    EXPECT_EQ(recv_empty, std::nullopt);
    auto req = s1.transferReq(false);
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at("a"), true);
}

TEST(FuncResultStoreTest, addResult) {
    auto data = std::make_shared<ClientData>("test");
    FuncResultStore &s = data->func_result_store;
    auto &r = s.addResult("a", Field{data, "b", "c"});
    EXPECT_EQ(r.name(), "c");
    EXPECT_EQ(r.member().name(), "b");
    EXPECT_EQ(&s.getResult(0), &r);
}

TEST(ClientDataTest, self) {
    ClientData data("test");
    EXPECT_TRUE(data.isSelf(FieldBase{"test"}));
    EXPECT_FALSE(data.isSelf(FieldBase{"a"}));
    EXPECT_FALSE(data.isSelf(FieldBase{""}));
}
TEST(ClientDataTest, memberIds) {
    ClientData data("test");
    data.member_ids["a"] = 1;
    EXPECT_EQ(data.getMemberNameFromId(1), "a");
    EXPECT_EQ(data.getMemberNameFromId(2), "");
    EXPECT_EQ(data.getMemberNameFromId(0), "");
    EXPECT_EQ(data.getMemberIdFromName("a"), 1);
    EXPECT_EQ(data.getMemberIdFromName("b"), 0);
    EXPECT_EQ(data.getMemberIdFromName(""), 0);
}

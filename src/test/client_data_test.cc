#include <gtest/gtest.h>
#include "../client/client_internal.h"
#include <webcface/member.h>
#include <string>

using namespace webcface;
using namespace webcface::Internal;

class SyncDataStore2Test : public ::testing::Test {
  protected:
    std::u8string self_name = u8"test";
    SyncDataStore2<std::string> s2{u8"test"};
};
TEST_F(SyncDataStore2Test, self) {
    EXPECT_TRUE(s2.isSelf(self_name));
    EXPECT_FALSE(s2.isSelf(u8"hoge"));
    EXPECT_FALSE(s2.isSelf(u8""));
}
TEST_F(SyncDataStore2Test, setSend) {
    s2.setSend(u8"a", "b");
    EXPECT_EQ(s2.getRecv(self_name, u8"a"), "b");
    auto send = s2.transferSend(false);
    EXPECT_EQ(send.at(u8"a"), "b");
    EXPECT_EQ(send.size(), 1);
    EXPECT_EQ(s2.transferSend(false).size(), 0);
    EXPECT_EQ(s2.transferSend(true).size(), 1);

    s2.setSend(u8"a", "b"); // 同じデータ
    EXPECT_EQ(s2.getRecv(self_name, u8"a"), "b");
    send = s2.transferSend(false);
    EXPECT_FALSE(send.count(u8"a"));
    EXPECT_EQ(send.size(), 0);
    EXPECT_EQ(s2.transferSend(true).size(), 1);

    s2.setSend(u8"a", "zzzzzz");
    EXPECT_EQ(s2.getRecv(self_name, u8"a"), "zzzzzz");
    s2.setSend(u8"a", "b"); // 一度違うデータを送ってから同じデータ
    EXPECT_EQ(s2.getRecv(self_name, u8"a"), "b");
    send = s2.transferSend(false);
    EXPECT_FALSE(send.count(u8"a"));
    EXPECT_EQ(send.size(), 0);
    EXPECT_EQ(s2.transferSend(true).size(), 1);

    s2.setSend(u8"a", "c"); // 違うデータ
    EXPECT_EQ(s2.getRecv(self_name, u8"a"), "c");
    send = s2.transferSend(false);
    EXPECT_EQ(send.at(u8"a"), "c");
    EXPECT_EQ(send.size(), 1);
    EXPECT_EQ(s2.transferSend(true).size(), 1);
}
// TEST_F(SyncDataStore2Test, setRecv) {}
TEST_F(SyncDataStore2Test, addReq) {
    auto reqi = s2.addReq(self_name, u8"b");
    EXPECT_EQ(reqi, 0);
    EXPECT_EQ(s2.getReq(1, u8"").first, u8"");
    EXPECT_EQ(s2.getReq(1, u8"").second, u8"");
    EXPECT_EQ(s2.transferReq().size(), 0);

    reqi = s2.addReq(u8"a", u8"b");
    EXPECT_EQ(reqi, 1);
    EXPECT_EQ(s2.getReq(1, u8"").first, u8"a");
    EXPECT_EQ(s2.getReq(1, u8"").second, u8"b");
    auto req = s2.transferReq();
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at(u8"a").at(u8"b"), 1);
    EXPECT_EQ(s2.transferReq().size(), 1);
}
TEST_F(SyncDataStore2Test, getRecv) {
    auto recv_empty = s2.getRecv(self_name, u8"b");
    EXPECT_EQ(recv_empty, std::nullopt);

    recv_empty = s2.getRecv(u8"a", u8"b");
    EXPECT_EQ(recv_empty, std::nullopt);

    s2.setRecv(u8"a", u8"b", "c");
    EXPECT_EQ(s2.getRecv(u8"a", u8"b"), "c");
}
TEST_F(SyncDataStore2Test, getRecvRecurse) {
    std::vector<std::u8string> cb_called;
    auto cb = [&](const std::u8string &f) { cb_called.push_back(f); };
    auto recv_empty = s2.getRecvRecurse(self_name, u8"b", cb);
    EXPECT_EQ(cb_called.size(), 0);
    EXPECT_EQ(recv_empty, std::nullopt);

    recv_empty = s2.getRecvRecurse(u8"a", u8"b", cb);
    EXPECT_EQ(cb_called.size(), 0);
    EXPECT_EQ(recv_empty, std::nullopt);

    s2.setRecv(u8"a", u8"b.a", "a");
    s2.setRecv(u8"a", u8"b.b", "b");
    recv_empty = s2.getRecvRecurse(u8"a", u8"b", cb);
    EXPECT_EQ(cb_called.size(), 2);
    EXPECT_EQ(std::count(cb_called.begin(), cb_called.end(), u8"b.a"), 1);
    EXPECT_EQ(std::count(cb_called.begin(), cb_called.end(), u8"b.b"), 1);
    EXPECT_EQ(recv_empty.value()["a"].get(), "a");
    EXPECT_EQ(recv_empty.value()["b"].get(), "b");
}
TEST_F(SyncDataStore2Test, unsetRecv) {
    auto reqi0 = s2.unsetRecv(u8"a", u8"b");
    EXPECT_FALSE(reqi0);

    s2.setRecv(u8"a", u8"b", "c");
    s2.addReq(u8"d", u8"e");
    auto reqi1 = s2.unsetRecv(u8"a", u8"b");
    auto reqi2 = s2.unsetRecv(u8"d", u8"e");
    EXPECT_FALSE(reqi1);
    EXPECT_TRUE(reqi2);
    EXPECT_EQ(s2.getReq(1, u8"").first, u8"");
    EXPECT_EQ(s2.getReq(1, u8"").second, u8"");
    EXPECT_EQ(s2.getRecv(u8"a", u8"b"), std::nullopt);
}
TEST_F(SyncDataStore2Test, clearRecv) {
    s2.setRecv(u8"a", u8"b", "c");
    s2.clearRecv(u8"a", u8"b");
    s2.clearRecv(u8"d", u8"e");
    EXPECT_EQ(s2.getRecv(u8"a", u8"b"), std::nullopt);
}
TEST_F(SyncDataStore2Test, setEntry) {
    s2.setEntry(u8"a", u8"b");
    EXPECT_EQ(s2.getEntry(u8"a").size(), 1);
    EXPECT_EQ(s2.getEntry(u8"a").count(u8"b"), 1);
}

class SyncDataStore1Test : public ::testing::Test {
  protected:
    std::u8string self_name = u8"test";
    SyncDataStore1<std::string> s1{u8"test"};
};
TEST_F(SyncDataStore1Test, self) {
    EXPECT_TRUE(s1.isSelf(self_name));
    EXPECT_FALSE(s1.isSelf(u8"hoge"));
    EXPECT_FALSE(s1.isSelf(u8""));
}
// TEST_F(SyncDataStore1Test, setRecv) {}
TEST_F(SyncDataStore1Test, addReq) {
    auto reqi = s1.addReq(self_name);
    EXPECT_FALSE(reqi);
    EXPECT_EQ(s1.transferReq().size(), 0);

    reqi = s1.addReq(u8"a");
    EXPECT_TRUE(reqi);
    auto req = s1.transferReq();
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at(u8"a"), true);
}
TEST_F(SyncDataStore1Test, clearReq) {
    auto reqi = s1.clearReq(u8"a");
    EXPECT_FALSE(reqi);

    s1.addReq(u8"a");
    reqi = s1.clearReq(u8"a");
    EXPECT_TRUE(reqi);
    auto req = s1.transferReq();
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at(u8"a"), false);
}
TEST_F(SyncDataStore1Test, getRecv) {
    auto recv_empty = s1.getRecv(self_name);
    EXPECT_EQ(recv_empty, std::nullopt);

    recv_empty = s1.getRecv(u8"a");
    EXPECT_EQ(recv_empty, std::nullopt);

    s1.setRecv(u8"a", "c");
    EXPECT_EQ(s1.getRecv(u8"a"), "c");
}

TEST(FuncResultStoreTest, addResult) {
    auto data = std::make_shared<ClientData>(u8"test");
    FuncResultStore &s = data->func_result_store;
    auto r = s.addResult(Field{data, u8"b", u8"c"});
    EXPECT_EQ(r.name(), "c");
    EXPECT_EQ(r.member().name(), "b");
}

TEST(ClientDataTest, self) {
    ClientData data(u8"test");
    EXPECT_TRUE(data.isSelf(FieldBase{u8"test"}));
    EXPECT_FALSE(data.isSelf(FieldBase{u8"a"}));
    EXPECT_FALSE(data.isSelf(FieldBase{u8""}));
}
TEST(ClientDataTest, memberIds) {
    ClientData data(u8"test");
    data.member_ids[u8"a"] = 1;
    EXPECT_EQ(data.getMemberNameFromId(1), u8"a");
    EXPECT_EQ(data.getMemberNameFromId(2), u8"");
    EXPECT_EQ(data.getMemberNameFromId(0), u8"");
    EXPECT_EQ(data.getMemberIdFromName(u8"a"), 1);
    EXPECT_EQ(data.getMemberIdFromName(u8"b"), 0);
    EXPECT_EQ(data.getMemberIdFromName(u8""), 0);
}

#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/member.h>
#include <string>

using namespace webcface;
using namespace webcface::internal;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString(encoding::castToU8(std::string_view(str, len)));
}

class SyncDataStore2Test : public ::testing::Test {
  protected:
    SharedString self_name = "test"_ss;
    SyncDataStore2<std::string> s2{"test"_ss};
};
TEST_F(SyncDataStore2Test, self) {
    EXPECT_TRUE(s2.isSelf(self_name));
    EXPECT_FALSE(s2.isSelf("hoge"_ss));
    EXPECT_FALSE(s2.isSelf(""_ss));
}
TEST_F(SyncDataStore2Test, setSend) {
    s2.setSend("a"_ss, "b");
    EXPECT_EQ(s2.getRecv(self_name, "a"_ss), "b");
    auto send = s2.transferSend(false);
    EXPECT_EQ(send.at("a"_ss), "b");
    EXPECT_EQ(send.size(), 1);
    EXPECT_EQ(s2.transferSend(false).size(), 0);
    EXPECT_EQ(s2.transferSend(true).size(), 1);

    s2.setSend("a"_ss, "b"); // 同じデータ
    EXPECT_EQ(s2.getRecv(self_name, "a"_ss), "b");
    send = s2.transferSend(false);
    EXPECT_FALSE(send.count("a"_ss));
    EXPECT_EQ(send.size(), 0);
    EXPECT_EQ(s2.transferSend(true).size(), 1);

    s2.setSend("a"_ss, "zzzzzz");
    EXPECT_EQ(s2.getRecv(self_name, "a"_ss), "zzzzzz");
    s2.setSend("a"_ss, "b"); // 一度違うデータを送ってから同じデータ
    EXPECT_EQ(s2.getRecv(self_name, "a"_ss), "b");
    send = s2.transferSend(false);
    EXPECT_FALSE(send.count("a"_ss));
    EXPECT_EQ(send.size(), 0);
    EXPECT_EQ(s2.transferSend(true).size(), 1);

    s2.setSend("a"_ss, "c"); // 違うデータ
    EXPECT_EQ(s2.getRecv(self_name, "a"_ss), "c");
    send = s2.transferSend(false);
    EXPECT_EQ(send.at("a"_ss), "c");
    EXPECT_EQ(send.size(), 1);
    EXPECT_EQ(s2.transferSend(true).size(), 1);
}
// TEST_F(SyncDataStore2Test, setRecv) {}
TEST_F(SyncDataStore2Test, addReq) {
    auto reqi = s2.addReq(self_name, "b"_ss);
    EXPECT_EQ(reqi, 0);
    EXPECT_EQ(s2.getReq(1, ""_ss).first, ""_ss);
    EXPECT_EQ(s2.getReq(1, ""_ss).second, ""_ss);
    EXPECT_EQ(s2.transferReq().size(), 0);

    reqi = s2.addReq("a"_ss, "b"_ss);
    EXPECT_EQ(reqi, 1);
    EXPECT_EQ(s2.getReq(1, ""_ss).first, "a"_ss);
    EXPECT_EQ(s2.getReq(1, ""_ss).second, "b"_ss);
    auto req = s2.transferReq();
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at("a"_ss).at("b"_ss), 1);
    EXPECT_EQ(s2.transferReq().size(), 1);
}
TEST_F(SyncDataStore2Test, getRecv) {
    auto recv_empty = s2.getRecv(self_name, "b"_ss);
    EXPECT_EQ(recv_empty, std::nullopt);

    recv_empty = s2.getRecv("a"_ss, "b"_ss);
    EXPECT_EQ(recv_empty, std::nullopt);

    s2.setRecv("a"_ss, "b"_ss, "c");
    EXPECT_EQ(s2.getRecv("a"_ss, "b"_ss), "c");
}
TEST_F(SyncDataStore2Test, unsetRecv) {
    auto reqi0 = s2.unsetRecv("a"_ss, "b"_ss);
    EXPECT_FALSE(reqi0);

    s2.setRecv("a"_ss, "b"_ss, "c");
    s2.addReq("d"_ss, "e"_ss);
    auto reqi1 = s2.unsetRecv("a"_ss, "b"_ss);
    auto reqi2 = s2.unsetRecv("d"_ss, "e"_ss);
    EXPECT_FALSE(reqi1);
    EXPECT_TRUE(reqi2);
    EXPECT_EQ(s2.getReq(1, ""_ss).first, ""_ss);
    EXPECT_EQ(s2.getReq(1, ""_ss).second, ""_ss);
    EXPECT_EQ(s2.getRecv("a"_ss, "b"_ss), std::nullopt);
}
TEST_F(SyncDataStore2Test, clearRecv) {
    s2.setRecv("a"_ss, "b"_ss, "c");
    s2.clearRecv("a"_ss, "b"_ss);
    s2.clearRecv("d"_ss, "e"_ss);
    EXPECT_EQ(s2.getRecv("a"_ss, "b"_ss), std::nullopt);
}
TEST_F(SyncDataStore2Test, setEntry) {
    s2.setEntry("a"_ss, "b"_ss);
    EXPECT_EQ(s2.getEntry("a"_ss).size(), 1);
    EXPECT_EQ(s2.getEntry("a"_ss).count("b"_ss), 1);
}

class SyncDataStore1Test : public ::testing::Test {
  protected:
    SharedString self_name = "test"_ss;
    SyncDataStore1<std::string> s1{"test"_ss};
};
TEST_F(SyncDataStore1Test, self) {
    EXPECT_TRUE(s1.isSelf(self_name));
    EXPECT_FALSE(s1.isSelf("hoge"_ss));
    EXPECT_FALSE(s1.isSelf(""_ss));
}
// TEST_F(SyncDataStore1Test, setRecv) {}
TEST_F(SyncDataStore1Test, addReq) {
    auto reqi = s1.addReq(self_name);
    EXPECT_FALSE(reqi);
    EXPECT_EQ(s1.transferReq().size(), 0);

    reqi = s1.addReq("a"_ss);
    EXPECT_TRUE(reqi);
    auto req = s1.transferReq();
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at("a"_ss), true);
}
TEST_F(SyncDataStore1Test, clearReq) {
    auto reqi = s1.clearReq("a"_ss);
    EXPECT_FALSE(reqi);

    s1.addReq("a"_ss);
    reqi = s1.clearReq("a"_ss);
    EXPECT_TRUE(reqi);
    auto req = s1.transferReq();
    EXPECT_EQ(req.size(), 1);
    EXPECT_EQ(req.at("a"_ss), false);
}
TEST_F(SyncDataStore1Test, getRecv) {
    auto recv_empty = s1.getRecv(self_name);
    EXPECT_EQ(recv_empty, std::nullopt);

    recv_empty = s1.getRecv("a"_ss);
    EXPECT_EQ(recv_empty, std::nullopt);

    s1.setRecv("a"_ss, "c");
    EXPECT_EQ(s1.getRecv("a"_ss), "c");
}

TEST(FuncResultStoreTest, addResult) {
    auto data = std::make_shared<ClientData>("test"_ss);
    FuncResultStore &s = data->func_result_store;
    auto r = s.addResult(Field{data, "b"_ss, "c"_ss});
    EXPECT_EQ(r->getter().name(), "c");
    EXPECT_EQ(r->getter().member().name(), "b");
}

TEST(ClientDataTest, self) {
    ClientData data("test"_ss);
    EXPECT_TRUE(data.isSelf(FieldBase{"test"_ss}));
    EXPECT_FALSE(data.isSelf(FieldBase{"a"_ss}));
    EXPECT_FALSE(data.isSelf(FieldBase{""_ss}));
}
TEST(ClientDataTest, memberIds) {
    ClientData data("test"_ss);
    data.member_ids["a"_ss] = 1;
    EXPECT_EQ(data.getMemberNameFromId(1), "a"_ss);
    EXPECT_EQ(data.getMemberNameFromId(2), ""_ss);
    EXPECT_EQ(data.getMemberNameFromId(0), ""_ss);
    EXPECT_EQ(data.getMemberIdFromName("a"_ss), 1);
    EXPECT_EQ(data.getMemberIdFromName("b"_ss), 0);
    EXPECT_EQ(data.getMemberIdFromName(""_ss), 0);
}

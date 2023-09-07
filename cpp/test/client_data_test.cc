#include <gtest/gtest.h>
#include <webcface/client_data.h>
#include <string>

using namespace WebCFace;
class SyncDataStore2Test : public ::testing::Test {
  protected:
    std::string self_name = "test";
    ClientData::SyncDataStore2<std::string> s2{"test"};
};

TEST_F(SyncDataStore2Test, self) {
    EXPECT_TRUE(s2.isSelf(self_name));
    EXPECT_FALSE(s2.isSelf("hoge"));
    EXPECT_FALSE(s2.isSelf(""));
}
TEST_F(SyncDataStore2Test, setSend) {
    s2.setSend("a", "b");
    EXPECT_EQ(s2.getRecv(self_name, "a"), "b");
    EXPECT_EQ(s2.transferSend(false).at("a"), "b");
}
TEST_F(SyncDataStore2Test, setHidden) {
    s2.setHidden("a", true);
    s2.setHidden("b", false);
    EXPECT_TRUE(s2.isHidden("a"));
    EXPECT_FALSE(s2.isHidden("b"));
    EXPECT_FALSE(s2.isHidden("c"));
}
TEST_F(SyncDataStore2Test, setRecv) {
    s2.setRecv("a", "b", "c");
    EXPECT_EQ(s2.getRecv("a", "b"), "c");
}
TEST_F(SyncDataStore2Test, getRecv) {
    auto recv_empty = s2.getRecv(self_name, "b");
    EXPECT_EQ(recv_empty, std::nullopt);
    EXPECT_EQ(s2.getReq(1).first, "");
    EXPECT_EQ(s2.getReq(1).second, "");
    EXPECT_EQ(s2.transferReq(false).size(), 0);
    
    recv_empty = s2.getRecv("a", "b");
    EXPECT_EQ(recv_empty, std::nullopt);
    EXPECT_EQ(s2.getReq(1).first, "a");
    EXPECT_EQ(s2.getReq(1).second, "b");
    EXPECT_EQ(s2.transferReq(false).at("a").at("b"), 1);
}
TEST_F(SyncDataStore2Test, unsetRecv) {
    s2.setRecv("a", "b", "c");
    s2.getRecv("d", "e");
    s2.unsetRecv("a", "b");
    s2.unsetRecv("d", "e");
    EXPECT_EQ(s2.getReq(1).first, "");
    EXPECT_EQ(s2.getReq(1).second, "");
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

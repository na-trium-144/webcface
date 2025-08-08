#include "test_common.h"
#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/internal/logger.h>

using namespace webcface;

class LoggerTest : public ::testing::Test {
  protected:
    void SetUp() override {
        data_ = std::make_shared<internal::ClientData>(self_name);
        data_->start();
    }
    void TearDown() override {
        data_->close();
        data_->join();
        data_.reset();
    }
    SharedString self_name = "test"_ss;
    std::shared_ptr<internal::ClientData> data_;
};

TEST_F(LoggerTest, loggerBuf) {
    LoggerBuf b(data_.get(), "buf"_ss);
    std::ostream os(&b);
    os << "a\nb" << std::endl;
    auto cerr_buf = std::cerr.rdbuf();
    std::cerr.rdbuf(&b);
    std::cerr << "c" << std::endl;
    auto ls = data_->log_store.getRecv(self_name, "buf"_ss);
    ASSERT_EQ((*ls)->data.size(), 3u);
    EXPECT_EQ((*ls)->data[0].level_, 2);
    EXPECT_EQ((*ls)->data[0].message_.u8StringView(), "a");
    EXPECT_EQ((*ls)->data[1].level_, 2);
    EXPECT_EQ((*ls)->data[1].message_.u8StringView(), "b");
    EXPECT_EQ((*ls)->data[2].level_, 2);
    EXPECT_EQ((*ls)->data[2].message_.u8StringView(), "c");
    std::cerr.rdbuf(cerr_buf);
}
TEST_F(LoggerTest, loggerBufW) {
    LoggerBufW b(data_.get(), "buf"_ss);
    std::wostream os(&b);
    os << L"a\nb" << std::endl;
    auto wcerr_buf = std::wcerr.rdbuf();
    std::wcerr.rdbuf(&b);
    std::wcerr << L"c" << std::endl;
    auto ls = data_->log_store.getRecv(self_name, "buf"_ss);
    ASSERT_EQ((*ls)->data.size(), 3u);
    EXPECT_EQ((*ls)->data[0].level_, 2);
    EXPECT_EQ((*ls)->data[0].message_.u8StringView(), "a");
    EXPECT_EQ((*ls)->data[1].level_, 2);
    EXPECT_EQ((*ls)->data[1].message_.u8StringView(), "b");
    EXPECT_EQ((*ls)->data[2].level_, 2);
    EXPECT_EQ((*ls)->data[2].message_.u8StringView(), "c");
    std::wcerr.rdbuf(wcerr_buf);
}

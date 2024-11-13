#include <gtest/gtest.h>
#include "webcface/internal/client_internal.h"
#include <webcface/internal/logger.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include "webcface/common/internal/message.h"

using namespace webcface;

static SharedString operator""_ss(const char *str, std::size_t len) {
    return SharedString::fromU8String(std::string_view(str, len));
}

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
    EXPECT_EQ((*ls)->data[0].message_.u8String(), "a");
    EXPECT_EQ((*ls)->data[1].level_, 2);
    EXPECT_EQ((*ls)->data[1].message_.u8String(), "b");
    EXPECT_EQ((*ls)->data[2].level_, 2);
    EXPECT_EQ((*ls)->data[2].message_.u8String(), "c");
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
    EXPECT_EQ((*ls)->data[0].message_.u8String(), "a");
    EXPECT_EQ((*ls)->data[1].level_, 2);
    EXPECT_EQ((*ls)->data[1].message_.u8String(), "b");
    EXPECT_EQ((*ls)->data[2].level_, 2);
    EXPECT_EQ((*ls)->data[2].message_.u8String(), "c");
    std::wcerr.rdbuf(wcerr_buf);
}

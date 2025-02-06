#include <gtest/gtest.h>
#include <webcface/common/encoding.h>

using namespace webcface;

TEST(EncodingTest, usingUTF8) {
    EXPECT_TRUE(usingUTF8());
    usingUTF8(false);
    EXPECT_FALSE(usingUTF8());
}
TEST(EncodingTest, SharedString) {
    auto ss = SharedString::fromU8String("test");
    EXPECT_EQ(ss.u8String(), "test");
    EXPECT_EQ(ss.count(), 1);
    auto ss2 = ss;
    EXPECT_EQ(ss.u8String(), "test");
    EXPECT_EQ(ss2.u8String(), "test");
    EXPECT_EQ(ss, ss2);
    EXPECT_EQ(ss.u8String().data(), ss2.u8String().data());
    EXPECT_EQ(ss.count(), 2);
    EXPECT_EQ(ss2.count(), 2);

    auto ss3 = SharedString::fromU8String("test");
    EXPECT_EQ(ss, ss3);
    EXPECT_NE(ss.u8String().data(), ss3.u8String().data());

    ss = nullptr;
    EXPECT_EQ(ss.u8String(), "");
    EXPECT_EQ(ss, SharedString());
    EXPECT_EQ(ss2.u8String(), "test");
    EXPECT_EQ(ss.count(), 0);
    EXPECT_EQ(ss2.count(), 1);
}
TEST(EncodingTest, encode) {
    std::string_view s = "Aあ";
    std::wstring_view w = L"Aあ";
    std::string_view u = "Aあ";
    std::string us(u.cbegin(), u.cend());
    usingUTF8(true);
    EXPECT_EQ(SharedString::encode(us).u8String(), u);
    EXPECT_EQ(SharedString::encode(w).u8String(), u);
    usingUTF8(false);
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    // webcfaceはutf8エンコーディングでビルドしてるのでsはANSIではない
    EXPECT_NE(SharedString::encode(s).u8String(), u);
#else
    // linux, macではutf8として扱われる
    EXPECT_EQ(SharedString::encode(s).u8String(), u);
#endif
    EXPECT_EQ(SharedString::encode(w).u8String(), u);
}
TEST(EncodingTest, decode) {
    std::string_view s = "Aあ";
    std::wstring_view w = L"Aあ";
    std::string_view u = "Aあ";
    std::string us(u.cbegin(), u.cend());
    usingUTF8(true);
    EXPECT_EQ(SharedString::fromU8String(u).decode(), us);
    EXPECT_EQ(SharedString::fromU8String(u).decodeW(), w);
    usingUTF8(false);
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    // webcfaceはutf8エンコーディングでビルドしてるのでsはANSIではない
    EXPECT_NE(SharedString::fromU8String(u).decode(), s);
#else
    // linux, macではutf8として扱われる
    EXPECT_EQ(SharedString::fromU8String(u).decode(), s);
#endif
    EXPECT_EQ(SharedString::fromU8String(u).decodeW(), w);
}

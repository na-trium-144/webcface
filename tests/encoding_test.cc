#include <gtest/gtest.h>
#include <webcface/encoding/encoding.h>

using namespace webcface;
using namespace webcface::encoding;

TEST(EncodingTest, usingUTF8) {
    EXPECT_TRUE(usingUTF8());
    usingUTF8(false);
    EXPECT_FALSE(usingUTF8());
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
#ifdef _WIN32
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
#ifdef _WIN32
    // webcfaceはutf8エンコーディングでビルドしてるのでsはANSIではない
    EXPECT_NE(SharedString::fromU8String(u).decode(), s);
#else
    // linux, macではutf8として扱われる
    EXPECT_EQ(SharedString::fromU8String(u).decode(), s);
#endif
    EXPECT_EQ(SharedString::fromU8String(u).decodeW(), w);
}

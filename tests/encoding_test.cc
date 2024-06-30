#include <gtest/gtest.h>
#include <webcface/encoding/encoding.h>

using namespace webcface;
using namespace webcface::Encoding;

TEST(EncodingTest, usingUTF8) {
    EXPECT_TRUE(usingUTF8());
    usingUTF8(false);
    EXPECT_FALSE(usingUTF8());
}
TEST(EncodingTest, encode) {
    std::string_view s = "Aあ";
    std::wstring_view w = L"Aあ";
    std::u8string_view u = u8"Aあ";
    std::string us(u.cbegin(), u.cend());
    usingUTF8(true);
    EXPECT_EQ(encode(us), u);
    EXPECT_EQ(encodeW(w), u);
    usingUTF8(false);
#ifdef _WIN32
    // webcfaceはutf8エンコーディングでビルドしてるのでsはANSIではない
    EXPECT_NE(encode(s), u);
#else
    // linux, macではutf8として扱われる
    EXPECT_EQ(encode(s), u);
#endif
    EXPECT_EQ(encodeW(w), u);
}
TEST(EncodingTest, decode) {
    std::string_view s = "Aあ";
    std::wstring_view w = L"Aあ";
    std::u8string_view u = u8"Aあ";
    std::string us(u.cbegin(), u.cend());
    usingUTF8(true);
    EXPECT_EQ(decode(u), us);
    EXPECT_EQ(decodeW(u), w);
    usingUTF8(false);
#ifdef _WIN32
    // webcfaceはutf8エンコーディングでビルドしてるのでsはANSIではない
    EXPECT_NE(decode(u), s);
#else
    // linux, macではutf8として扱われる
    EXPECT_EQ(decode(u), s);
#endif
    EXPECT_EQ(decodeW(u), w);
}
TEST(EncodingTest, cast) {
    std::u8string_view u = u8"Aあ";
    std::string us(u.cbegin(), u.cend());
    EXPECT_EQ(castToU8(us), u);
    EXPECT_EQ(castFromU8(u), us);
}

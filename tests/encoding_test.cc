#include <gtest/gtest.h>
#include <webcface/common/encoding.h>

using namespace webcface;

TEST(EncodingTest, usingUTF8) {
    EXPECT_TRUE(usingUTF8());
    usingUTF8(false);
    EXPECT_FALSE(usingUTF8());
}
TEST(EncodingTest, encode) {
    std::string s = "Aあ";
    std::wstring w = L"Aあ";
    std::string u = "Aあ";
    usingUTF8(true);
    EXPECT_EQ(SharedString::encode(u).u8StringView(), u);
    EXPECT_EQ(SharedString::encode(w).u8StringView(), u);
    usingUTF8(false);
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    // webcfaceはutf8エンコーディングでビルドしてるのでsはANSIではない
    EXPECT_NE(SharedString::encode(s).u8StringView(), u);
#else
    // linux, macではutf8として扱われる
    EXPECT_EQ(SharedString::encode(s).u8StringView(), u);
#endif
    EXPECT_EQ(SharedString::encode(w).u8StringView(), u);
}
TEST(EncodingTest, decode) {
    std::string s = "Aあ";
    std::wstring w = L"Aあ";
    std::string u = "Aあ";
    usingUTF8(true);
    EXPECT_EQ(SharedString::fromU8String(u).decode(), u);
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

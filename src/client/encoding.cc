#include <webcface/encoding.h>
#include <webcface/common/def.h>
#include <utf8.h>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

WEBCFACE_NS_BEGIN
namespace Encoding {
static bool using_utf8 = true;

void usingUTF8(bool flag) { using_utf8 = flag; }
bool usingUTF8() { return using_utf8; }

std::u8string encode(std::string_view name) {
#ifdef _WIN32
    if (!using_utf8) {
        auto length = MultiByteToWideChar(
            CP_ACP, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_ACP, 0, name.data(),
                            static_cast<int>(name.size()), result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        return encodeW(result_utf16);
    }
#endif
    // そのままコピー
    return std::u8string(name.cbegin(), name.cend());
}

std::u8string encodeW(std::wstring_view name) {
#ifdef _WIN32
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length_utf8 = WideCharToMultiByte(CP_UTF8, 0, name.data(),
                                           static_cast<int>(name.size()),
                                           nullptr, 0, nullptr, nullptr);
    std::u8string result_utf8(length_utf8, '\0');
    auto result_ptr =
        static_cast<char *>(static_cast<void *>(result_utf8.data()));
    WideCharToMultiByte(CP_UTF8, 0, name.data(), static_cast<int>(name.size()),
                        result_ptr, static_cast<int>(result_utf8.size()),
                        nullptr, nullptr);
    return result_utf8;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::u8string result_utf8;
    utf8::utf32to8(name.cbegin(), name.cend(), std::back_inserter(result_utf8));
    return result_utf8;
#endif
}

std::string toNarrow(std::wstring_view name) {
#ifdef _WIN32
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length_utf8 = WideCharToMultiByte(
        using_utf8 ? CP_UTF8 : CP_ACP, 0, name.data(),
        static_cast<int>(name.size()), nullptr, 0, nullptr, nullptr);
    std::string result(length_utf8, '\0');
    WideCharToMultiByte(using_utf8 ? CP_UTF8 : CP_ACP, 0, name.data(),
                        static_cast<int>(name.size()), result.data(),
                        static_cast<int>(result.size()), nullptr, nullptr);
    return result;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::string result_utf8;
    utf8::utf32to8(name.cbegin(), name.cend(), std::back_inserter(result_utf8));
    return result_utf8;
#endif
}

std::wstring decodeW(std::u8string_view name_ref) {
#ifdef _WIN32
    auto name_ptr =
        static_cast<const char *>(static_cast<const void *>(name_ref.data()));
    auto length = MultiByteToWideChar(
        CP_UTF8, 0, name_ptr, static_cast<int>(name_ref.size()), nullptr, 0);
    std::wstring result_utf16(length, '\0');
    MultiByteToWideChar(CP_UTF8, 0, name_ptr, static_cast<int>(name_ref.size()),
                        result_utf16.data(),
                        static_cast<int>(result_utf16.length()));
    return result_utf16;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::wstring result_utf32;
    utf8::utf8to32(name_ref.cbegin(), name_ref.cend(),
                   std::back_inserter(result_utf32));
    return result_utf32;
#endif
}

std::wstring toWide(std::string_view name_ref) {
#ifdef _WIN32
    auto length =
        MultiByteToWideChar(using_utf8 ? CP_UTF8 : CP_ACP, 0, name_ref.data(),
                            static_cast<int>(name_ref.size()), nullptr, 0);
    std::wstring result_utf16(length, '\0');
    MultiByteToWideChar(using_utf8 ? CP_UTF8 : CP_ACP, 0, name_ref.data(),
                        static_cast<int>(name_ref.size()), result_utf16.data(),
                        static_cast<int>(result_utf16.length()));
    return result_utf16;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::wstring result_utf32;
    utf8::utf8to32(name_ref.cbegin(), name_ref.cend(),
                   std::back_inserter(result_utf32));
    return result_utf32;
#endif
}

std::string decode(std::u8string_view name_ref) {
#ifdef _WIN32
    if (!using_utf8) {
        auto result_utf16 = decodeW(name_ref);
        auto length_acp =
            WideCharToMultiByte(CP_ACP, 0, result_utf16.data(),
                                static_cast<int>(result_utf16.length()),
                                nullptr, 0, nullptr, nullptr);
        std::string result_acp(length_acp, '\0');
        WideCharToMultiByte(
            CP_ACP, 0, result_utf16.data(),
            static_cast<int>(result_utf16.length()), result_acp.data(),
            static_cast<int>(result_acp.size()), nullptr, nullptr);
        return result_acp;
    }
#endif
    // そのままコピー
    return std::string(name_ref.cbegin(), name_ref.cend());
}

std::u8string_view castToU8(std::string_view name) {
    return std::u8string_view(
        static_cast<const char8_t *>(static_cast<const void *>(name.data())),
        name.size());
}
std::string_view castFromU8(std::u8string_view name) {
    return std::string_view(
        static_cast<const char *>(static_cast<const void *>(name.data())),
        name.size());
}

} // namespace Encoding
WEBCFACE_NS_END

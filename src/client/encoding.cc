#include <webcface/char.h>
#include <webcface/common/def.h>
#include <utf8.h>
#include <cstring>

#ifdef _WIN32
#include <stringapiset.h>
#endif

WEBCFACE_NS_BEGIN
namespace Encoding {
static bool using_utf8 = true;

void usingUTF8(bool flag) { using_utf8 = flag; }
bool usingUTF8() { return using_utf8; }

std::vector<char> initName(const std::string &name) {
#ifdef _WIN32
    if (!using_utf8) {
        auto length = MultiByteToWideChar(
            CP_ACP, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_ACP, 0, name.data(),
                            static_cast<int>(name.size()), result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        return initNameW(result_utf16);
    }
#endif
    // そのままコピー
    std::vector<char> result_utf8(name.cbegin(), name.cend());
    result_utf8.push_back('\0');
    return result_utf8;
}

std::vector<char> initNameW(const std::wstring &name) {
#ifdef _WIN32
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length_utf8 = WideCharToMultiByte(CP_UTF8, 0, name.data(),
                                           static_cast<int>(name.size()),
                                           nullptr, 0, nullptr, nullptr);
    std::vector<char> result_utf8(length_utf8);
    WideCharToMultiByte(CP_UTF8, 0, name.data(), static_cast<int>(name.size()),
                        result_utf8.data(),
                        static_cast<int>(result_utf8.size()), nullptr, nullptr);
    return result_utf8;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::vector<char> result_utf8;
    utf8::utf32to8(name.cbegin(), name.cend(), std::back_inserter(result_utf8));
    result_utf8.push_back('\0');
    return result_utf8;
#endif
}

std::wstring getNameW(const void *name_ref) {
    int len = static_cast<int>(std::strlen(static_cast<const char *>(name_ref)));
#ifdef _WIN32
    auto length = MultiByteToWideChar(CP_UTF8, 0, static_cast<const char *>(name_ref), len, nullptr, 0);
    std::wstring result_utf16(length, '\0');
    MultiByteToWideChar(CP_UTF8, 0, name_ref, len, result_utf16.data(),
                        static_cast<int>(result_utf16.length()));
    return result_utf16;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::wstring result_utf32;
    utf8::utf8to32(static_cast<const char *>(name_ref), static_cast<const char *>(name_ref) + len, std::back_inserter(result_utf32));
    return result_utf32;
#endif
}

std::string getName(const void *name_ref) {
#ifdef _WIN32
    if (!using_utf8) {
        auto result_utf16 = getNameW(static_cast<const char *>(name_ref));
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
    return std::string(static_cast<const char *>(name_ref));
}

} // namespace Encoding
WEBCFACE_NS_END

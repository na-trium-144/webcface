#include "webcface/common/encoding.h"
#include <utf8.h>
#include <cstring>
#include <mutex>

#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
#include <windows.h>
#endif

WEBCFACE_NS_BEGIN

namespace internal {
struct SharedStringData {
    std::string u8s;
    std::string s;
    std::wstring ws;
    std::recursive_mutex m;
    explicit SharedStringData(std::string_view u8s)
        : u8s(u8s), s(), ws(), m() {}
    SharedStringData(std::string_view u8s, std::string_view s,
                     std::wstring_view ws)
        : u8s(u8s), s(s), ws(ws), m() {}
};

} // namespace internal

/// \private
static bool using_utf8 = true;

void usingUTF8(bool flag) { using_utf8 = flag; }
bool usingUTF8() { return using_utf8; }

const std::string &SharedString::emptyStr() {
    static std::string empty;
    return empty;
}
const std::wstring &SharedString::emptyStrW() {
    static std::wstring empty;
    return empty;
}

SharedString::SharedString(std::shared_ptr<internal::SharedStringData> &&data)
    : data(std::move(data)) {}
bool SharedString::operator==(const SharedString &other) const {
    return this->data == other.data || this->u8String() == other.u8String();
}
bool SharedString::operator<=(const SharedString &other) const {
    return this->data == other.data || this->u8String() <= other.u8String();
}
bool SharedString::operator>=(const SharedString &other) const {
    return this->data == other.data || this->u8String() >= other.u8String();
}
bool SharedString::operator!=(const SharedString &other) const {
    return this->data != other.data && this->u8String() != other.u8String();
}
bool SharedString::operator<(const SharedString &other) const {
    return this->data != other.data && this->u8String() < other.u8String();
}
bool SharedString::operator>(const SharedString &other) const {
    return this->data != other.data && this->u8String() > other.u8String();
}

SharedString SharedString::fromU8String(std::string_view u8s) {
    return SharedString(std::make_shared<internal::SharedStringData>(u8s));
}
SharedString SharedString::encode(std::string_view name) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        auto length = MultiByteToWideChar(
            CP_ACP, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_ACP, 0, name.data(),
                            static_cast<int>(name.size()), result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        return encode(result_utf16, name);
    }
#endif
    // そのままコピー
    return fromU8String(name);
}
SharedString SharedString::encode(std::wstring_view name, std::string_view s) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length_utf8 = WideCharToMultiByte(CP_UTF8, 0, name.data(),
                                           static_cast<int>(name.size()),
                                           nullptr, 0, nullptr, nullptr);
    std::string result_utf8(length_utf8, '\0');
    WideCharToMultiByte(CP_UTF8, 0, name.data(), static_cast<int>(name.size()),
                        result_utf8.data(),
                        static_cast<int>(result_utf8.size()), nullptr, nullptr);
    return SharedString(
        std::make_shared<internal::SharedStringData>(result_utf8, s, name));
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::string result_utf8;
    utf8::utf32to8(name.cbegin(), name.cend(), std::back_inserter(result_utf8));
    return SharedString(
        std::make_shared<internal::SharedStringData>(result_utf8, s, name));
#endif
}


const std::string &SharedString::u8String() const {
    if (!data) {
        return emptyStr();
    } else {
        return data->u8s;
    }
}
std::string_view SharedString::u8StringView() const {
    if (!data) {
        return std::string_view();
    } else {
        return data->u8s;
    }
}
bool SharedString::empty() const { return u8String().empty(); }
bool SharedString::startsWith(std::string_view str) const {
    return u8StringView().substr(0, str.size()) == str;
}
bool SharedString::startsWith(char str) const {
    return !u8StringView().empty() && u8StringView()[0] == str;
}
SharedString SharedString::substr(std::size_t pos, std::size_t len) const {
    if (!data) {
        return *this;
    } else {
        return SharedString::fromU8String(u8StringView().substr(pos, len));
    }
}
std::size_t SharedString::find(char c, std::size_t pos) const {
    if (!data) {
        return std::string::npos;
    } else {
        return u8StringView().find(c, pos);
    }
}

std::string toNarrow(std::wstring_view name) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
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

const std::wstring &SharedString::decodeW() const {
    if (!data || data->u8s.empty()) {
        return emptyStrW();
    } else {
        std::lock_guard lock(data->m);
        if (!data->ws.empty()) {
            return data->ws;
        } else {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
            static_assert(sizeof(wchar_t) == 2,
                          "Assuming wchar_t is utf-16 on Windows");
            auto length = MultiByteToWideChar(
                CP_UTF8, 0, data->u8s.data(),
                static_cast<int>(data->u8s.size()), nullptr, 0);
            std::wstring result_utf16(length, '\0');
            MultiByteToWideChar(CP_UTF8, 0, data->u8s.data(),
                                static_cast<int>(data->u8s.size()),
                                result_utf16.data(),
                                static_cast<int>(result_utf16.length()));
            data->ws = result_utf16;
            return data->ws;
#else
            static_assert(sizeof(wchar_t) == 4,
                          "Assuming wchar_t is utf-32 on Unix");
            std::wstring result_utf32;
            utf8::utf8to32(data->u8s.cbegin(), data->u8s.cend(),
                           std::back_inserter(result_utf32));
            data->ws = result_utf32;
            return data->ws;
#endif
        }
    }
}

std::wstring toWide(std::string_view name_ref) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
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

const std::string &SharedString::decode() const {
    if (!data || data->u8s.empty()) {
        return emptyStr();
    } else {
        std::lock_guard lock(data->m);
        if (!data->s.empty()) {
            return data->s;
        } else {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
            if (!using_utf8) {
                auto result_utf16 = decodeW();
                auto length_acp =
                    WideCharToMultiByte(CP_ACP, 0, result_utf16.data(),
                                        static_cast<int>(result_utf16.length()),
                                        nullptr, 0, nullptr, nullptr);
                std::string result_acp(length_acp, '\0');
                WideCharToMultiByte(
                    CP_ACP, 0, result_utf16.data(),
                    static_cast<int>(result_utf16.length()), result_acp.data(),
                    static_cast<int>(result_acp.size()), nullptr, nullptr);
                data->s = result_acp;
                return data->s;
            }
#endif
            // そのままコピー
            return data->u8s;
        }
    }
}

WEBCFACE_NS_END

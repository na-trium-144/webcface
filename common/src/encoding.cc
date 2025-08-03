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
    std::variant<std::string, std::string_view> u8s;
    std::variant<std::string, std::string_view> s;
    std::variant<std::wstring, std::wstring_view> ws;
    std::recursive_mutex m;
    explicit SharedStringData(std::string u8s)
        : u8s(std::move(u8s)), s(std::string()), ws(std::wstring()), m() {}
    explicit SharedStringData(const char *u8s, std::size_t N)
        : u8s(std::string_view(u8s, N - 1)), s(std::string()), ws(std::wstring()), m() {}
    SharedStringData(std::string u8s, std::string s,
                     std::wstring ws)
        : u8s(std::move(u8s)), s(std::move(s)), ws(std::move(ws)), m() {}
    SharedStringData(std::string u8s, const char *s, std::size_t N,
                     std::wstring ws)
        : u8s(std::move(u8s)), s(std::string_view(s, N - 1)), ws(std::move(ws)), m() {}
    SharedStringData(std::string u8s, std::string s,
                     const wchar_t *ws, std::size_t N)
        : u8s(std::move(u8s)), s(std::move(s)), ws(std::wstring_view(ws, N - 1)), m() {}
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

SharedString SharedString::fromU8String(std::string u8s) {
    return SharedString(std::make_shared<internal::SharedStringData>(std::move(u8s)));
}
SharedString SharedString::fromU8StringStatic(const char *u8s, std::size_t N) {
    return SharedString(std::make_shared<internal::SharedStringData>(u8s, N));
}
SharedString SharedString::encode(std::string name) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        auto length = MultiByteToWideChar(
            CP_ACP, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_ACP, 0, name.data(),
                            static_cast<int>(name.size()), result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        // return encode(std::move(result_utf16), std::move(name));
        auto length_utf8 = WideCharToMultiByte(CP_UTF8, 0, result_utf16.data(),
                                               static_cast<int>(result_utf16.size()),
                                               nullptr, 0, nullptr, nullptr);
        std::string result_utf8(length_utf8, '\0');
        WideCharToMultiByte(CP_UTF8, 0, result_utf16.data(), static_cast<int>(result_utf16.size()),
                            result_utf8.data(),
                            static_cast<int>(result_utf8.size()), nullptr, nullptr);
        return SharedString(
            std::make_shared<internal::SharedStringData>(std::move(result_utf8), std::move(name), std::move(result_utf16)));
    }
#endif
    // そのままコピー
    return fromU8String(std::move(name));
}
SharedString SharedString::encodeStatic(const char *name, std::size_t N) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        auto length = MultiByteToWideChar(
            CP_ACP, 0, name, static_cast<int>(N-1), nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_ACP, 0, name,
                            static_cast<int>(N-1), result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        // return encodeStatic2(std::move(result_utf16), name);
        auto length_utf8 = WideCharToMultiByte(CP_UTF8, 0, result_utf16.data(),
                                               static_cast<int>(result_utf16.size()),
                                               nullptr, 0, nullptr, nullptr);
        std::string result_utf8(length_utf8, '\0');
        WideCharToMultiByte(CP_UTF8, 0, result_utf16.data(), static_cast<int>(result_utf16.size()),
                            result_utf8.data(),
                            static_cast<int>(result_utf8.size()), nullptr, nullptr);
        return SharedString(
            std::make_shared<internal::SharedStringData>(std::move(result_utf8), name, N, std::move(result_utf16)));
    }
#endif
    // そのままコピー
    return fromU8StringStatic(name, N);
}
SharedString SharedString::encode(std::wstring name) {
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
        std::make_shared<internal::SharedStringData>(std::move(result_utf8), std::string(), std::move(name)));
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::string result_utf8;
    utf8::utf32to8(name.cbegin(), name.cend(), std::back_inserter(result_utf8));
    return SharedString(
        std::make_shared<internal::SharedStringData>(std::move(result_utf8), std::string(), std::move(name)));
#endif
}
SharedString SharedString::encodeStatic(const wchar_t *name, std::size_t N) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length_utf8 = WideCharToMultiByte(CP_UTF8, 0, name,
                                           static_cast<int>(N-1),
                                           nullptr, 0, nullptr, nullptr);
    std::string result_utf8(length_utf8, '\0');
    WideCharToMultiByte(CP_UTF8, 0, name, static_cast<int>(N-1),
                        result_utf8.data(),
                        static_cast<int>(result_utf8.size()), nullptr, nullptr);
    return SharedString(
        std::make_shared<internal::SharedStringData>(std::move(result_utf8), std::string(), name, N));
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::string result_utf8;
    utf8::utf32to8(name, name + N, std::back_inserter(result_utf8));
    return SharedString(
        std::make_shared<internal::SharedStringData>(std::move(result_utf8), std::string(), name, N));
#endif
}


std::string_view SharedString::u8StringView() const {
    if (!data) {
        return std::string_view();
    } else {
        switch(data->u8s.index()){
        case 0:
            return std::get<0>(data->u8s);
        default:
            return std::get<1>(data->u8s);
        }
    }
}
bool SharedString::empty() const { return u8StringView().empty(); }
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

std::wstring_view SharedString::decodeW() const {
    if (!data || empty()) {
        return emptyStrW();
    } else {
        std::lock_guard lock(data->m);
        switch(data->ws.index()){
        case 0:
            if(!std::get<0>(data->ws).empty()){
                return std::get<0>(data->ws);
            }
        default:
            if(!std::get<1>(data->ws).empty()){
                return std::get<1>(data->ws);
            }
        }
        std::string_view u8s = u8StringView();
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
        static_assert(sizeof(wchar_t) == 2,
                      "Assuming wchar_t is utf-16 on Windows");
        auto length = MultiByteToWideChar(
            CP_UTF8, 0, u8s.data(),
            static_cast<int>(u8s.size()), nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_UTF8, 0, u8s.data(),
                            static_cast<int>(u8s.size()),
                            result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        data->ws.emplace<std::wstring>(std::move(result_utf16));
        return data->ws;
#else
        static_assert(sizeof(wchar_t) == 4,
                      "Assuming wchar_t is utf-32 on Unix");
        std::wstring result_utf32;
        utf8::utf8to32(u8s.cbegin(), u8s.cend(),
                       std::back_inserter(result_utf32));
        data->ws.emplace<std::wstring>(std::move(result_utf32));
        return data->ws;
#endif
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

std::string_view SharedString::decode() const {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        if (!data || empty()) {
            return emptyStr();
        } else {
            std::lock_guard lock(data->m);
            switch(data->s.index()){
            case 0:
                if(!std::get<0>(data->s).empty()){
                    return std::get<0>(data->s);
                }
            default:
                if(!std::get<1>(data->s).empty()){
                    return std::get<1>(data->s);
                }
            }
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
            data->s.emplace<std::string>(std::move(result_acp));
            return data->s;
        }
    }
#endif
    // そのままコピー
    return u8StringView();
}

WEBCFACE_NS_END

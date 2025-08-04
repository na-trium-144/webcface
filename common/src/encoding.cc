#include "webcface/common/encoding.h"
#include <utf8.h>
#include <cstring>
#include <mutex>
#include <cassert>

#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
#include <windows.h>
#endif

WEBCFACE_NS_BEGIN

template <typename CharT>
TStringView<CharT>::TStringView(const CharT *data, std::size_t size,
                                const std::basic_string<CharT> *container)
    : std::basic_string_view<CharT>(data, size), c_str_(data),
      container_(container) {
    assert(data[size] == static_cast<CharT>(0));
    if (container_) {
        assert(container->c_str() == data);
    }
}
template class WEBCFACE_DLL_INSTANCE_DEF TStringView<char>;
template class WEBCFACE_DLL_INSTANCE_DEF TStringView<wchar_t>;

namespace internal {

struct SharedStringData {
    std::variant<std::string, StringView> u8s;
    std::variant<std::string, StringView> s;
    std::variant<std::wstring, WStringView> ws;
    std::recursive_mutex m;

    explicit SharedStringData(std::string u8s)
        : u8s(std::move(u8s)), s(std::string()), ws(std::wstring()), m() {}
    explicit SharedStringData(StringView u8s)
        : u8s(u8s), s(std::string()), ws(std::wstring()), m() {}
    SharedStringData(std::string u8s, std::string s, std::wstring ws)
        : u8s(std::move(u8s)), s(std::move(s)), ws(std::move(ws)), m() {}
    SharedStringData(std::string u8s, StringView s, std::wstring ws)
        : u8s(std::move(u8s)), s(s), ws(std::move(ws)), m() {}
    SharedStringData(std::string u8s, std::string s, WStringView ws)
        : u8s(std::move(u8s)), s(std::move(s)), ws(ws), m() {}

    StringView u8sv() const {
        switch (u8s.index()) {
        case 0:
            return StringView(std::get<0>(u8s).c_str(), std::get<0>(u8s).size(),
                              &std::get<0>(u8s));
        default:
            return std::get<1>(u8s);
        }
    }
    StringView sv() const {
        switch (s.index()) {
        case 0:
            return StringView(std::get<0>(s).c_str(), std::get<0>(s).size(),
                              &std::get<0>(s));
        default:
            return std::get<1>(s);
        }
    }
    WStringView wsv() const {
        switch (ws.index()) {
        case 0:
            return WStringView(std::get<0>(ws).c_str(), std::get<0>(ws).size(),
                               &std::get<0>(ws));
        default:
            return std::get<1>(ws);
        }
    }
};

} // namespace internal

/// \private
static bool using_utf8 = true;

void usingUTF8(bool flag) { using_utf8 = flag; }
bool usingUTF8() { return using_utf8; }

StringView SharedString::emptyStr() {
    static std::string empty;
    return StringView(empty.c_str(), empty.size(), &empty);
}
WStringView SharedString::emptyStrW() {
    static std::wstring empty;
    return WStringView(empty.c_str(), empty.size(), &empty);
}

SharedString::SharedString(std::shared_ptr<internal::SharedStringData> &&data)
    : data(std::move(data)) {}
bool SharedString::operator==(const SharedString &other) const {
    return this->data == other.data ||
           this->u8StringView() == other.u8StringView();
}
bool SharedString::operator<=(const SharedString &other) const {
    return this->data == other.data ||
           this->u8StringView() <= other.u8StringView();
}
bool SharedString::operator>=(const SharedString &other) const {
    return this->data == other.data ||
           this->u8StringView() >= other.u8StringView();
}

SharedString SharedString::fromU8String(std::string u8s) {
    return SharedString(
        std::make_shared<internal::SharedStringData>(std::move(u8s)));
}
SharedString SharedString::fromU8StringStatic(StringView u8s) {
    return SharedString(std::make_shared<internal::SharedStringData>(u8s));
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
        auto length_utf8 =
            WideCharToMultiByte(CP_UTF8, 0, result_utf16.data(),
                                static_cast<int>(result_utf16.size()), nullptr,
                                0, nullptr, nullptr);
        std::string result_utf8(length_utf8, '\0');
        WideCharToMultiByte(
            CP_UTF8, 0, result_utf16.data(),
            static_cast<int>(result_utf16.size()), result_utf8.data(),
            static_cast<int>(result_utf8.size()), nullptr, nullptr);
        return SharedString(std::make_shared<internal::SharedStringData>(
            std::move(result_utf8), std::move(name), std::move(result_utf16)));
    }
#endif
    // そのままコピー
    return fromU8String(std::move(name));
}
SharedString SharedString::encodeStatic(StringView name) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        auto length = MultiByteToWideChar(
            CP_ACP, 0, name.data(), static_cast<int>(name.size()), nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_ACP, 0, name.data(),
                            static_cast<int>(name.size()), result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        auto length_utf8 =
            WideCharToMultiByte(CP_UTF8, 0, result_utf16.data(),
                                static_cast<int>(result_utf16.size()), nullptr,
                                0, nullptr, nullptr);
        std::string result_utf8(length_utf8, '\0');
        WideCharToMultiByte(
            CP_UTF8, 0, result_utf16.data(),
            static_cast<int>(result_utf16.size()), result_utf8.data(),
            static_cast<int>(result_utf8.size()), nullptr, nullptr);
        return SharedString(std::make_shared<internal::SharedStringData>(
            std::move(result_utf8), name, std::move(result_utf16)));
    }
#endif
    // そのままコピー
    return fromU8StringStatic(name);
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
    return SharedString(std::make_shared<internal::SharedStringData>(
        std::move(result_utf8), std::string(), std::move(name)));
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::string result_utf8;
    utf8::utf32to8(name.cbegin(), name.cend(), std::back_inserter(result_utf8));
    return SharedString(std::make_shared<internal::SharedStringData>(
        std::move(result_utf8), std::string(), std::move(name)));
#endif
}
SharedString SharedString::encodeStatic(WStringView name) {
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
    return SharedString(std::make_shared<internal::SharedStringData>(
        std::move(result_utf8), std::string(), name));
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::string result_utf8;
    utf8::utf32to8(name.begin(), name.end(), std::back_inserter(result_utf8));
    return SharedString(std::make_shared<internal::SharedStringData>(
        std::move(result_utf8), std::string(), name));
#endif
}


StringView SharedString::u8StringView() const {
    if (!data) {
        return emptyStr();
    } else {
        return data->u8sv();
    }
}
bool SharedString::empty() const { return !data || u8StringView().size() == 0; }
bool SharedString::startsWith(std::string_view str) const {
    return u8StringView().substr(0, str.size()) == str;
}
bool SharedString::startsWith(char str) const {
    return !empty() && u8StringView().front() == str;
}
SharedString SharedString::substr(std::size_t pos, std::size_t len) const {
    if (!data) {
        return *this;
    } else {
        return SharedString::fromU8String(
            std::string(u8StringView().substr(pos, len)));
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

WStringView SharedString::decodeW() const {
    if (!data || empty()) {
        return emptyStrW();
    } else {
        std::lock_guard lock(data->m);
        if (data->wsv().empty()) {
            auto u8s = u8StringView();
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
            static_assert(sizeof(wchar_t) == 2,
                          "Assuming wchar_t is utf-16 on Windows");
            auto length =
                MultiByteToWideChar(CP_UTF8, 0, u8s.data(),
                                    static_cast<int>(u8s.size()), nullptr, 0);
            std::wstring result_utf16(length, '\0');
            MultiByteToWideChar(
                CP_UTF8, 0, u8s.data(), static_cast<int>(u8s.size()),
                result_utf16.data(), static_cast<int>(result_utf16.length()));
            data->ws.emplace<std::wstring>(std::move(result_utf16));
#else
            static_assert(sizeof(wchar_t) == 4,
                          "Assuming wchar_t is utf-32 on Unix");
            std::wstring result_utf32;
            utf8::utf8to32(u8s.begin(), u8s.end(),
                           std::back_inserter(result_utf32));
            data->ws.emplace<std::wstring>(std::move(result_utf32));
#endif
        }
        return data->wsv();
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

StringView SharedString::decode() const {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        if (!data || empty()) {
            return emptyStr();
        } else {
            std::lock_guard lock(data->m);
            if (data->sv().empty()) {
                auto result_utf16 = decodeW();
                auto length_acp =
                    WideCharToMultiByte(CP_ACP, 0, result_utf16.data(),
                                        static_cast<int>(result_utf16.size()),
                                        nullptr, 0, nullptr, nullptr);
                std::string result_acp(length_acp, '\0');
                WideCharToMultiByte(
                    CP_ACP, 0, result_utf16.data(),
                    static_cast<int>(result_utf16.size()), result_acp.data(),
                    static_cast<int>(result_acp.size()), nullptr, nullptr);
                data->s.emplace<std::string>(std::move(result_acp));
            }
            return data->sv();
        }
    }
#endif
    // そのままコピー
    return u8StringView();
}

WEBCFACE_NS_END

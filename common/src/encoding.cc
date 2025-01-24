#include "webcface/common/encoding.h"
#include <cassert>
#include <utf8.h>
#include <cstring>
#include <mutex>
#include <atomic>

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
    std::atomic<int> count = 1;
    SharedStringData(std::string u8s, std::string s = {},
                     std::wstring ws = {}) noexcept
        : u8s(std::move(u8s)), s(std::move(s)), ws(std::move(ws)) {}
    void inc() noexcept {
        assert(count > 0);
        count++;
    }
    void dec() noexcept {
        if (--count == 0) {
            delete this;
        }
    }
};

} // namespace internal

/// \private
static bool using_utf8 = true;

void usingUTF8(bool flag) noexcept { using_utf8 = flag; }
bool usingUTF8() noexcept { return using_utf8; }

SharedString::CStrView<char> SharedString::emptyStr() noexcept {
    static std::string empty;
    return {empty.c_str(), 0};
}
SharedString::CStrView<wchar_t> SharedString::emptyStrW() noexcept {
    static std::wstring empty;
    return {empty.c_str(), 0};
}

SharedString::SharedString(const SharedString &other) noexcept
    : data(other.data) {
    if (data) {
        data->inc();
    }
}
SharedString &SharedString::operator=(const SharedString &other) noexcept {
    if (this != &other) {
        if (data) {
            data->dec();
        }
        data = other.data;
        if (data) {
            data->inc();
        }
    }
    return *this;
}
SharedString::~SharedString() noexcept {
    if (data) {
        data->dec();
    }
}
SharedString::SharedString(SharedString &&other) noexcept : data(other.data) {
    other.data = nullptr;
}
SharedString &SharedString::operator=(SharedString &&other) noexcept {
    if (this != &other) {
        if (data) {
            data->dec();
        }
        data = other.data;
        other.data = nullptr;
    }
    return *this;
}
SharedString::SharedString(internal::SharedStringData *&&data) noexcept
    : data(data) {
    assert(!data || data->count == 1);
}

int SharedString::count() const noexcept {
    if (data) {
        assert(data->count > 0);
        return data->count.load();
    } else {
        return 0;
    }
}

SharedString SharedString::fromU8String(const char *u8s, std::size_t len) {
    return SharedString(new internal::SharedStringData(std::string(u8s, len)));
}
SharedString SharedString::encode(const char *s, std::size_t len) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        auto length = MultiByteToWideChar(CP_ACP, 0, s, static_cast<int>(len),
                                          nullptr, 0);
        std::wstring result_utf16(length, '\0');
        MultiByteToWideChar(CP_ACP, 0, s, static_cast<int>(len),
                            result_utf16.data(),
                            static_cast<int>(result_utf16.length()));
        return encode(result_utf16, name);
    }
#endif
    // そのままコピー
    return fromU8String(s, len);
}
SharedString SharedString::encode(const wchar_t *ws, std::size_t wlen,
                                  const char *s, std::size_t slen) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length_utf8 = WideCharToMultiByte(
        CP_UTF8, 0, ws, static_cast<int>(wlen), nullptr, 0, nullptr, nullptr);
    std::string result_utf8(length_utf8, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws, static_cast<int>(wlen),
                        result_utf8.data(),
                        static_cast<int>(result_utf8.size()), nullptr, nullptr);
    return SharedString(new internal::SharedStringData(
        std::move(result_utf8), s ? std::string(s, slen) : {},
        std::wstring(ws, wlen)));
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    std::string result_utf8;
    utf8::utf32to8(ws, ws + wlen, std::back_inserter(result_utf8));
    return SharedString(new internal::SharedStringData(
        std::move(result_utf8), s ? std::string(s, slen) : std::string{},
        std::wstring(ws, wlen)));
#endif
}


SharedString::CStrView<char> SharedString::u8CStr() const noexcept {
    if (!data) {
        return emptyStr();
    } else {
        return {data->u8s.c_str(), data->u8s.size()};
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

SharedString::CStrView<wchar_t> SharedString::cDecodeW() const {
    if (!data || data->u8s.empty()) {
        return emptyStrW();
    } else {
        std::lock_guard lock(data->m);
        if (!data->ws.empty()) {
            return {data->ws.c_str(), data->ws.size()};
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
            return {data->ws.c_str(), data->ws.size()};
#else
            static_assert(sizeof(wchar_t) == 4,
                          "Assuming wchar_t is utf-32 on Unix");
            std::wstring result_utf32;
            utf8::utf8to32(data->u8s.cbegin(), data->u8s.cend(),
                           std::back_inserter(result_utf32));
            data->ws = result_utf32;
            return {data->ws.c_str(), data->ws.size()};
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

SharedString::CStrView<char> SharedString::cDecode() const {
    if (!data || data->u8s.empty()) {
        return emptyStr();
    } else {
        std::lock_guard lock(data->m);
        if (!data->s.empty()) {
            return {data->s.c_str(), data->s.size()};
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
                return {data->s.c_str(), data->s.size()};
            }
#endif
            // そのままコピー
            return {data->u8s.c_str(), data->u8s.size()};
        }
    }
}

WEBCFACE_NS_END

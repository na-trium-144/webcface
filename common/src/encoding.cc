#include "webcface/common/encoding.h"
#include "webcface/common/internal/safe_global.h"
#include <unordered_map>
#include <utf8.h>
#include <cstring>
#include <mutex>
#include <cassert>

#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
#include <windows.h>
#endif

WEBCFACE_NS_BEGIN

namespace internal {

struct SharedStringData {
    std::string u8s;
    std::string s;
    std::wstring ws;
    std::string_view u8sv;
    std::string_view sv;
    std::wstring_view wsv;
    std::recursive_mutex m;

    explicit SharedStringData(std::string u8s)
        : u8s(std::move(u8s)), u8sv(this->u8s), sv(this->s), wsv(this->ws) {}
    explicit SharedStringData(std::string_view u8sv)
        : u8sv(u8sv), sv(this->s), wsv(this->ws) {
        assert(*(u8sv.data() + u8sv.size()) == '\0');
    }
    SharedStringData(std::nullptr_t, std::string s)
        : s(std::move(s)), u8sv(this->u8s), sv(this->s), wsv(this->ws) {}
    SharedStringData(std::nullptr_t, std::string_view sv)
        : u8sv(this->u8s), sv(sv), wsv(this->ws) {
        assert(*(sv.data() + sv.size()) == '\0');
    }
    SharedStringData(std::nullptr_t, std::nullptr_t, std::wstring ws)
        : ws(std::move(ws)), u8sv(this->u8s), sv(this->s), wsv(this->ws) {}
    SharedStringData(std::nullptr_t, std::nullptr_t, std::wstring_view wsv)
        : u8sv(this->u8s), sv(this->s), wsv(wsv) {
        assert(*(wsv.data() + wsv.size()) == L'\0');
    }
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

SharedString::SharedString(std::shared_ptr<internal::SharedStringData> data)
    : data(std::move(data)) {}

/// \private
void encodeWsToU8s(const std::shared_ptr<internal::SharedStringData> &data) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length_utf8 = WideCharToMultiByte(CP_UTF8, 0, data->wsv.data(),
                                           static_cast<int>(data->wsv.size()),
                                           nullptr, 0, nullptr, nullptr);
    assert(data->u8s.empty() && data->u8sv.empty());
    data->u8s.assign(length_utf8, '\0');
    WideCharToMultiByte(CP_UTF8, 0, data->wsv.data(),
                        static_cast<int>(data->wsv.size()), data->u8s.data(),
                        static_cast<int>(data->u8s.size()), nullptr, nullptr);
    data->u8sv = data->u8s;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    assert(data->u8s.empty() && data->u8sv.empty());
    utf8::utf32to8(data->wsv.cbegin(), data->wsv.cend(),
                   std::back_inserter(data->u8s));
    data->u8sv = data->u8s;
#endif
}
/// \private
void encodeSToWs(
    [[maybe_unused]] const std::shared_ptr<internal::SharedStringData> &data) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    auto length =
        MultiByteToWideChar(CP_ACP, 0, data->sv.data(),
                            static_cast<int>(data->sv.size()), nullptr, 0);
    assert(data->ws.empty() && data->wsv.empty());
    data->ws.assign(length, '\0');
    MultiByteToWideChar(CP_ACP, 0, data->sv.data(),
                        static_cast<int>(data->sv.size()), data->ws.data(),
                        static_cast<int>(data->ws.length()));
    data->wsv = data->ws;
#else
    assert(false && "encodeImpl is not implemented on this platform");
#endif
}

SharedString SharedString::fromU8String(std::string u8s) {
    return SharedString(
        std::make_shared<internal::SharedStringData>(std::move(u8s)));
}
SharedString SharedString::fromU8StringStatic(std::string_view u8s) {
    // ポインタの場合、shared_ptrを作るよりunordered_mapから探したほうが速い
    static internal::SafeGlobal<std::unordered_map<
        const char *, std::shared_ptr<internal::SharedStringData>>>
        literal_caches;
    if (literal_caches && literal_caches->count(u8s.data()) > 0) {
        return SharedString(literal_caches->at(u8s.data()));
    }
    auto data = std::make_shared<internal::SharedStringData>(u8s);
    if (literal_caches) {
        literal_caches->emplace(u8s.data(), data);
    }
    return SharedString(data);
}

SharedString SharedString::encode(std::string name) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        // auto encode_result = encodeImpl(name);
        return SharedString(std::make_shared<internal::SharedStringData>(
            nullptr, std::move(name)));
    }
#endif
    // そのままコピー
    return fromU8String(std::move(name));
}
SharedString SharedString::encodeStatic(std::string_view name) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        static internal::SafeGlobal<std::unordered_map<
            const char *, std::shared_ptr<internal::SharedStringData>>>
            literal_caches;
        if (literal_caches && literal_caches->count(name.data()) > 0) {
            return SharedString(literal_caches->at(name.data()));
        }
        // auto encode_result = encodeImpl(name);
        auto data = std::make_shared<internal::SharedStringData>(nullptr, name);
        if (literal_caches) {
            literal_caches->emplace(name.data(), data);
        }
        return SharedString(data);
    }
#endif
    // そのままコピー
    return fromU8StringStatic(name);
}

SharedString SharedString::encode(std::wstring name) {
    // auto result_utf8 = encodeImplW(name);
    return SharedString(std::make_shared<internal::SharedStringData>(
        nullptr, nullptr, std::move(name)));
}
SharedString SharedString::encodeStatic(std::wstring_view name) {
    static internal::SafeGlobal<std::unordered_map<
        const wchar_t *, std::shared_ptr<internal::SharedStringData>>>
        literal_caches;
    if (literal_caches && literal_caches->count(name.data()) > 0) {
        return SharedString(literal_caches->at(name.data()));
    }
    // auto result_utf8 = encodeImplW(name);
    auto data =
        std::make_shared<internal::SharedStringData>(nullptr, nullptr, name);
    if (literal_caches) {
        literal_caches->emplace(name.data(), data);
    }
    return SharedString(data);
}

bool SharedString::operator==(const SharedString &other) const {
    if (empty()) {
        return other.empty();
    } else if (other.empty()) {
        return false; // return empty();
    } else {
        // data != nullptr
        if (!data->wsv.empty() || !other.data->wsv.empty()) {
            return decodeW() == other.decodeW();
        } else if (!data->u8sv.empty() || !other.data->u8sv.empty()) {
            return u8StringView() == other.u8StringView();
        } else {
            return decode() == other.decode();
        }
    }
}
bool SharedString::operator<=(const SharedString &other) const {
    return u8StringView() <= other.u8StringView();
}
bool SharedString::operator>=(const SharedString &other) const {
    return u8StringView() >= other.u8StringView();
}

std::string_view SharedString::u8StringView() const {
    if (!data) {
        // null終端を保証する必要はある
        return StringView{};
    } else {
        std::lock_guard lock(data->m);
        if (data->u8sv.empty() && !data->wsv.empty()) {
            encodeWsToU8s(data);
        } else if (data->u8sv.empty() && !data->sv.empty()) {
            encodeSToWs(data);
            encodeWsToU8s(data);
        }
        return data->u8sv;
    }
}
StringView SharedString::u8StringViewShare() const {
    if (!data) {
        return {};
    } else {
        std::lock_guard lock(data->m);
        if (data->u8sv.empty() && !data->wsv.empty()) {
            encodeWsToU8s(data);
        } else if (data->u8sv.empty() && !data->sv.empty()) {
            encodeSToWs(data);
            encodeWsToU8s(data);
        }
        return StringView(data->u8sv.data(), data->u8sv.size(), data);
    }
}
bool SharedString::empty() const {
    return !data ||
           (data->u8sv.empty() && data->sv.empty() && data->wsv.empty());
}
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

void decodeU8sToWs(const std::shared_ptr<internal::SharedStringData> &data) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    static_assert(sizeof(wchar_t) == 2,
                  "Assuming wchar_t is utf-16 on Windows");
    auto length =
        MultiByteToWideChar(CP_UTF8, 0, data->u8sv.data(),
                            static_cast<int>(data->u8sv.size()), nullptr, 0);
    assert(data->ws.empty() && data->wsv.empty());
    data->ws.assign(length, '\0');
    MultiByteToWideChar(CP_UTF8, 0, data->u8sv.data(),
                        static_cast<int>(data->u8sv.size()), data->ws.data(),
                        static_cast<int>(data->ws.length()));
    data->wsv = data->ws;
#else
    static_assert(sizeof(wchar_t) == 4, "Assuming wchar_t is utf-32 on Unix");
    assert(data->ws.empty());
    utf8::utf8to32(data->u8sv.begin(), data->u8sv.end(),
                   std::back_inserter(data->ws));
    data->wsv = data->ws;
#endif
}
void decodeWsToS(
    [[maybe_unused]] const std::shared_ptr<internal::SharedStringData> &data) {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    auto length_acp = WideCharToMultiByte(CP_ACP, 0, data->wsv.data(),
                                          static_cast<int>(data->wsv.size()),
                                          nullptr, 0, nullptr, nullptr);
    assert(data->s.empty() && data->sv.empty());
    data->s.assign(length_acp, '\0');
    WideCharToMultiByte(CP_ACP, 0, data->wsv.data(),
                        static_cast<int>(data->wsv.size()), data->s.data(),
                        static_cast<int>(data->s.size()), nullptr, nullptr);
    data->sv = data->s;
#else
    assert(false && "decodeImpl is not implemented on this platform");
#endif
}

std::wstring_view SharedString::decodeW() const {
    if (!data) {
        return WStringView{};
    } else {
        std::lock_guard lock(data->m);
        if (data->wsv.empty() && !data->u8sv.empty()) {
            decodeU8sToWs(data);
        } else if (data->wsv.empty() && !data->sv.empty()) {
            encodeSToWs(data);
        }
        return data->wsv;
    }
}
WStringView SharedString::decodeShareW() const {
    if (!data) {
        return {};
    } else {
        std::lock_guard lock(data->m);
        if (data->wsv.empty() && !data->u8sv.empty()) {
            decodeU8sToWs(data);
        } else if (data->wsv.empty() && !data->sv.empty()) {
            encodeSToWs(data);
        }
        return WStringView(data->wsv.data(), data->wsv.size(), data);
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
        if (!data) {
            return StringView{};
        } else {
            std::lock_guard lock(data->m);
            if (data->sv.empty() && !data->wsv.empty()) {
                decodeWsToS(data);
            } else if (data->sv.empty() && !data->u8sv.empty()) {
                decodeU8sToWs(data);
                decodeWsToS(data);
            }
            return data->sv;
        }
    }
#endif
    // そのままコピー
    return u8StringView();
}
StringView SharedString::decodeShare() const {
#if WEBCFACE_SYSTEM_WCHAR_WINDOWS
    if (!using_utf8) {
        if (!data) {
            return {};
        } else {
            std::lock_guard lock(data->m);
            if (data->sv.empty() && !data->wsv.empty()) {
                decodeWsToS(data);
            } else if (data->sv.empty() && !data->u8sv.empty()) {
                decodeU8sToWs(data);
                decodeWsToS(data);
            }
            return StringView(data->sv.data(), data->sv.size(), data);
        }
    }
#endif
    // そのままコピー
    return u8StringViewShare();
}

WEBCFACE_NS_END

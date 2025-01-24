#pragma once
#include <map>
#include <set>
#include <string>
#include <string_view>
#include "c_encoding.h"

WEBCFACE_NS_BEGIN
namespace internal {
struct SharedStringData;
} // namespace internal

/*!
 * \brief webcfaceが使用するエンコーディングを設定する
 * \since ver2.0
 *
 * * windowsでは、falseの場合webcfaceの各種クラスのインタフェースで使われる
 * std::string をすべてANSIエンコーディングとみなし、
 * 内部でutf8と相互変換する。
 * * デフォルトは true (以前のバージョンとの互換性のため)
 * * unixでは効果がない(この設定に関わらず文字列はすべてutf8とみなされ相互変換は行われない)
 * * std::wstringには影響しないが、windowsで loggerWStreamBuf(),
 * loggerWOStream()
 * を使用する場合は出力するコンソールのコードページに合わせること
 *
 */
void usingUTF8(bool flag) noexcept { wcfUsingUTF8(flag); }
/*!
 * \brief webcfaceが使用するエンコーディングを取得する
 * \since ver2.0
 *
 */
bool usingUTF8() noexcept { return wcfGetUsingUTF8(); }

/*!
 * \brief stringをwstringに変換する
 * \since ver2.0
 */
std::wstring toWide(std::string_view name_ref);
/*!
 * \brief wstringをstringに変換する
 * \since ver2.0
 */
std::string toNarrow(std::wstring_view name_ref);

/*!
 * \brief u8stringとstringとwstringをshared_ptrで持ち共有する
 * \since ver2.0
 *
 * * 初期状態ではdataがnullptr、またはu8stringのみ値を持ちstringとwstringは空
 * * コピーするとdataのポインタ(shared_ptr)のみをコピーし、
 * 文字列自体のコピーは発生しない
 * * SharedStringどうしを比較するときdataポインタが等しければ文字列自体の比較をしない。
 * * decodeやdecodeWが呼ばれたときdata内部に変換後の文字列を保存する。
 * 一度保存したstringやwstringを別の値に書き換えることはない
 * (のでc_strなどの参照は保持される)
 * * string→utf8:
 * windowsではusingUTF8(false)の場合ANSIからutf8へエンコーディングの変換を行うが、
 * usingUTF8(true)の場合なにもせずそのままコピーする。
 * * utf-8→string: windowsでusingUTF8(false)の場合はANSIに、
 * それ以外の場合なにもせずそのままコピーする。
 * * (ver3.0〜) u8String(), decode(), decodeW()
 * はstringではなくstring_viewに変更
 *   * ただしいずれもdata()が返すポインタがnull終端であることが保証される
 *
 */
class SharedString {
    internal::SharedStringData *data;

    static WEBCFACE_DLL SharedString WEBCFACE_CALL
    fromU8String(const char *u8s, std::size_t len);
    static WEBCFACE_DLL SharedString WEBCFACE_CALL encode(const char *s,
                                                          std::size_t len);
    static WEBCFACE_DLL SharedString WEBCFACE_CALL encode(const wchar_t *ws,
                                                          std::size_t wlen,
                                                          const char *s,
                                                          std::size_t slen);

    /*!
     * \brief 文字列を参照する構造体
     * \since ver3.0
     *
     * data はnull終端された文字列、
     * size はnull終端を含まない文字列の長さ
     */
    template <typename CharT>
    struct CStrView {
        const CharT *data;
        std::size_t size;
        operator std::basic_string_view<CharT>() const { return {data, size}; }
    };

    WEBCFACE_DLL CStrView<char> u8CStr() const noexcept;
    WEBCFACE_DLL CStrView<wchar_t> cDecodeW() const;
    WEBCFACE_DLL CStrView<char> cDecode() const;

  public:
    SharedString(std::nullptr_t = nullptr) : data() {}
    WEBCFACE_DLL SharedString(const SharedString &other) noexcept;
    WEBCFACE_DLL SharedString &operator=(const SharedString &other) noexcept;
    WEBCFACE_DLL SharedString(SharedString &&other) noexcept;
    WEBCFACE_DLL SharedString &operator=(SharedString &&other) noexcept;
    WEBCFACE_DLL ~SharedString() noexcept;
    explicit WEBCFACE_DLL
    SharedString(internal::SharedStringData *&&data) noexcept;

    WEBCFACE_DLL int count() const noexcept;

    static SharedString fromU8String(std::string_view u8s) {
        return fromU8String(u8s.data(), u8s.size());
    }
    static SharedString encode(std::string_view s) {
        return encode(s.data(), s.size());
    }
    static SharedString encode(std::wstring_view ws) {
        return encode(ws.data(), ws.size(), nullptr, 0);
    }

    std::string_view u8String() const noexcept { return u8CStr(); }
    std::string_view u8StringView() const noexcept { return u8CStr(); }
    std::string_view decode() const noexcept { return cDecode(); }
    std::wstring_view decodeW() const noexcept { return cDecodeW(); }

    static WEBCFACE_DLL CStrView<char> WEBCFACE_CALL emptyStr() noexcept;
    static WEBCFACE_DLL CStrView<wchar_t> WEBCFACE_CALL emptyStrW() noexcept;

    bool empty() const { return u8String().empty(); }
    bool startsWith(std::string_view str) const {
        return u8String().substr(0, str.size()) == str;
    }
    bool startsWith(char str) const {
        return !empty() && u8String().front() == str;
    }
    SharedString substr(std::size_t pos,
                        std::size_t len = std::string::npos) const {
        if (!data) {
            return *this;
        } else {
            return SharedString::fromU8String(u8String().substr(pos, len));
        }
    }
    std::size_t find(char c, std::size_t pos = 0) const {
        return u8String().find(c, pos);
    }

    bool operator==(const SharedString &other) const noexcept {
        return data == other.data || u8String() == other.u8String();
    }
    bool operator<=(const SharedString &other) const noexcept {
        return data == other.data || u8String() <= other.u8String();
    }
    bool operator>=(const SharedString &other) const noexcept {
        return data == other.data || u8String() >= other.u8String();
    }
    bool operator!=(const SharedString &other) const noexcept {
        return !(*this == other);
    }
    bool operator<(const SharedString &other) const noexcept {
        return !(*this >= other);
    }
    bool operator>(const SharedString &other) const noexcept {
        return !(*this <= other);
    }

    bool operator==(std::string_view other) const noexcept {
        return u8String() == other;
    }
    bool operator<=(std::string_view other) const noexcept {
        return u8String() <= other;
    }
    bool operator>=(std::string_view other) const noexcept {
        return u8String() >= other;
    }
    bool operator!=(std::string_view other) const noexcept {
        return !(*this == other);
    }
    bool operator<(std::string_view other) const noexcept {
        return !(*this >= other);
    }
    bool operator>(std::string_view other) const noexcept {
        return !(*this <= other);
    }
};

template <typename T>
using StrMap1 = std::map<SharedString, T, std::less<>>;
template <typename T>
using StrMap2 = StrMap1<StrMap1<T>>;
using StrSet1 = std::set<SharedString, std::less<>>;
using StrSet2 = StrMap1<StrSet1>;

namespace [[deprecated("symbols in webcface::encoding namespace are "
                       "now directly in webcface namespace")]] encoding {
inline bool usingUTF8() { return webcface::usingUTF8(); }
inline void usingUTF8(bool flag) { webcface::usingUTF8(flag); }

using SharedString = webcface::SharedString;

template <typename T>
using StrMap1 = webcface::StrMap1<T>;
template <typename T>
using StrMap2 = webcface::StrMap2<T>;
using StrSet1 = webcface::StrSet1;
using StrSet2 = webcface::StrSet2;

} // namespace encoding

WEBCFACE_NS_END

#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <cassert>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

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
WEBCFACE_DLL void WEBCFACE_CALL usingUTF8(bool flag);
/*!
 * \brief webcfaceが使用するエンコーディングを取得する
 * \since ver2.0
 *
 */
WEBCFACE_DLL bool WEBCFACE_CALL usingUTF8();

/*!
 * \brief stringをwstringに変換する
 * \since ver2.0
 */
WEBCFACE_DLL std::wstring WEBCFACE_CALL toWide(std::string_view name_ref);
/*!
 * \brief wstringをstringに変換する
 * \since ver2.0
 */
WEBCFACE_DLL std::string WEBCFACE_CALL toNarrow(std::wstring_view name_ref);

/*!
 * \brief null終端であることが保証されたstring_view
 * \since ver2.10
 */
template <typename CharT>
struct TerminatedStringView {
    const CharT *c_str;
    std::size_t size;
    const std::basic_string<CharT> *container;
    TerminatedStringView(const CharT *data, std::size_t size,
                         const std::basic_string<CharT> *container)
        : c_str(data), size(size), container(container) {
        assert(data[size] == static_cast<CharT>(0));
        if (container) {
            assert(container->c_str() == data);
        }
    }
    std::basic_string_view<CharT> std() const {
        return std::basic_string_view<CharT>(c_str, size);
    }
};

/*!
 * \brief u8stringとstringとwstringをshared_ptrで持ち共有する
 * \since ver2.0
 * \sa String
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
 * * ver2.10〜
 * staticな生文字列ポインタをstring_viewとして保持することを可能にした。
 * その場合string_viewの範囲外だがNULL終端であることが保証される。
 *
 */
class WEBCFACE_DLL SharedString {
    std::shared_ptr<internal::SharedStringData> data;

  public:
    SharedString() : data() {}
    SharedString(std::nullptr_t) : data() {}
    explicit SharedString(std::shared_ptr<internal::SharedStringData> &&data);

    static SharedString WEBCFACE_CALL fromU8String(std::string u8s);
    static SharedString WEBCFACE_CALL
    fromU8StringStatic(TerminatedStringView<char> u8s);
    static SharedString WEBCFACE_CALL encode(std::string s);
    static SharedString WEBCFACE_CALL
    encodeStatic(TerminatedStringView<char> s);
    static SharedString WEBCFACE_CALL encode(std::wstring ws);
    static SharedString WEBCFACE_CALL
    encodeStatic(TerminatedStringView<wchar_t> ws);

    TerminatedStringView<char> u8StringView() const;
    TerminatedStringView<char> decode() const;
    TerminatedStringView<wchar_t> decodeW() const;

    static TerminatedStringView<char> emptyStr();
    static TerminatedStringView<wchar_t> emptyStrW();

    bool empty() const;
    bool startsWith(std::string_view str) const;
    bool startsWith(char str) const;
    SharedString substr(std::size_t pos,
                        std::size_t len = std::string::npos) const;
    std::size_t find(char c, std::size_t pos = 0) const;

    bool operator==(const SharedString &other) const;
    bool operator<=(const SharedString &other) const;
    bool operator>=(const SharedString &other) const;
    bool operator!=(const SharedString &other) const {
        return !(*this == other);
    }
    bool operator<(const SharedString &other) const {
        return !(*this >= other);
    }
    bool operator>(const SharedString &other) const {
        return !(*this <= other);
    }
};

/*!
 * \brief SharedString のpublicなコンストラクタインタフェース (入力専用)
 * \since ver2.10
 *
 * * stringまたはwstringを受け取り、保持する
 * * windowsではusingUTF8(false)の場合毎回ANSIからutf8へエンコーディングの変換を行うが、
 * usingUTF8(true)の場合なにもせずそのままコピーする。
 * * 生文字列リテラルを渡した場合に限り、コピーせずポインタで保持する。
 *
 */
class String : public SharedString {
  public:
    String() : SharedString() {}
    String(std::string s) : SharedString(SharedString::encode(std::move(s))) {}
    String(std::wstring s) : SharedString(SharedString::encode(std::move(s))) {}
    template <std::size_t N>
    String(const char (&static_str)[N])
        : SharedString(SharedString::encodeStatic(
              TerminatedStringView<char>(static_str, N - 1, nullptr))) {}
    template <std::size_t N>
    String(const wchar_t (&static_str)[N])
        : SharedString(SharedString::encodeStatic(
              TerminatedStringView<wchar_t>(static_str, N - 1, nullptr))) {}
};

/*!
 * \brief string_viewやconst char*同士を連結しstringを返す
 * \since ver2.10
 *
 * String, SharedString, TerminatedStringView
 * などと同じヘッダーにあるが、それらとはなんの関係もない。
 *
 */
template <typename CharT, typename... Args>
std::basic_string<CharT> strJoin(std::basic_string_view<CharT> first_str,
                                 Args &&...args) {
    std::basic_string<CharT> str;
    str.reserve(first_str.size() +
                (std::basic_string_view<CharT>(args).size() + ...));
    str.append(first_str);
    (str.append(std::basic_string_view<CharT>(args)), ...);
    return str;
}

namespace [[deprecated("symbols in webcface::encoding namespace are "
                       "now directly in webcface namespace")]] encoding {
inline bool usingUTF8() { return webcface::usingUTF8(); }
inline void usingUTF8(bool flag) { webcface::usingUTF8(flag); }
inline std::wstring toWide(std::string_view name_ref) {
    return webcface::toWide(name_ref);
}
inline std::string toNarrow(std::wstring_view name) {
    return webcface::toNarrow(name);
}

using SharedString = webcface::SharedString;

} // namespace encoding

WEBCFACE_NS_END

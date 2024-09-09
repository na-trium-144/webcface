#pragma once
#include <map>
#include <set>
#include <string>
#include <string_view>
#include <memory>
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN
namespace internal {
struct SharedStringData;
} // namespace internal

inline namespace encoding {
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
 *
 */
class WEBCFACE_DLL SharedString {
    std::shared_ptr<internal::SharedStringData> data;

  public:
    SharedString() : data() {}
    SharedString(std::nullptr_t) : data() {}
    explicit SharedString(std::shared_ptr<internal::SharedStringData> &&data);

    static SharedString WEBCFACE_CALL fromU8String(std::string_view u8s);
    static SharedString WEBCFACE_CALL encode(std::string_view s);
    static SharedString WEBCFACE_CALL
    encode(std::wstring_view ws, std::string_view s = std::string_view());

    const std::string &u8String() const;
    std::string_view u8StringView() const;
    const std::string &decode() const;
    const std::wstring &decodeW() const;

    static const std::string &emptyStr();
    static const std::wstring &emptyStrW();

    bool empty() const;
    bool startsWith(std::string_view str) const;
    bool startsWith(char str) const;

    bool operator==(const SharedString &other) const;
    bool operator<=(const SharedString &other) const;
    bool operator>=(const SharedString &other) const;
    bool operator!=(const SharedString &other) const;
    bool operator<(const SharedString &other) const;
    bool operator>(const SharedString &other) const;

    struct Hash : std::hash<std::string> {
        Hash() = default;
        auto operator()(const SharedString &ss) const {
            return this->std::hash<std::string>::operator()(ss.u8String());
        }
    };
};

template <typename T>
using StrMap1 = std::map<SharedString, T>;
template <typename T>
using StrMap2 = StrMap1<StrMap1<T>>;
using StrSet1 = std::set<SharedString>;
using StrSet2 = StrMap1<StrSet1>;

} // namespace encoding
WEBCFACE_NS_END

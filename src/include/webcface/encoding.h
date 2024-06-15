#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
inline namespace Encoding {
/*!
 * \brief webcfaceが使用するエンコーディングを設定する
 * \since ver2.0
 *
 * * windowsでは、falseの場合webcfaceの各種クラスのインタフェースで使われる
 * std::string をすべてANSIエンコーディングとみなし、
 * 内部でutf8と相互変換する。
 * * デフォルトは true (以前のバージョンとの互換性のため)
 * * unixでは効果がない。
 * * std::wstring をspdlogに渡して使用する場合、内部で
 * wstring→utf-8 の変換がされる場合があるのでtrueにすることを推奨
 * * spdlogのloggerではなく Client::loggerWStreamBuf() を使用する場合は、
 * 出力するコンソールのコードページに合わせた設定にする必要がある
 *
 */
WEBCFACE_DLL void usingUTF8(bool flag);
/*!
 * \brief webcfaceが使用するエンコーディングを取得する
 * \since ver2.0
 *
 */
WEBCFACE_DLL bool usingUTF8();

/*!
 * \brief stringをutf8のchar配列に変換する
 * \since ver2.0
 *
 * windowsではusingUTF8(false)の場合ANSIからutf8へエンコーディングの変換を行うが、
 * usingUTF8(true)の場合なにもせずそのままコピーする。
 *
 * utf8であることを明確にするために戻り値型をu8stringにしている
 *
 */
WEBCFACE_DLL std::u8string encode(std::string_view name);

/*!
 * \brief wstringをutf8のchar配列に変換する
 * \since ver2.0
 *
 * (unixではたぶん必要ないが、#ifで分岐するのがめんどいので作ってしまう)
 *
 * utf8であることを明確にするために戻り値型をu8stringにしている
 *
 */
WEBCFACE_DLL std::u8string encodeW(std::wstring_view name);

/*!
 * \brief utf8の文字列をstringに変換する
 * \since ver2.0
 *
 * windowsでusingUTF8(false)の場合はANSIに、
 * それ以外の場合なにもせずそのままコピーする。
 *
 */
WEBCFACE_DLL std::string decode(std::u8string_view name_ref);
/*!
 * \brief utf8の文字列をwstringに変換する
 * \since ver2.0
 */
WEBCFACE_DLL std::wstring decodeW(std::u8string_view name_ref);

/*!
 * \brief stringをwstringに変換する
 * \since ver2.0
 */
WEBCFACE_DLL std::wstring toWide(std::string_view name_ref);
/*!
 * \brief wstringをstringに変換する
 * \since ver2.0
 */
WEBCFACE_DLL std::string toNarrow(std::wstring_view name_ref);

/*!
 * \brief stringをu8stringにキャストする
 * \since ver2.0
 */
WEBCFACE_DLL std::u8string_view castToU8(std::string_view name);
/*!
 * \since ver2.0
 */
inline std::u8string_view castToU8(const char *data, std::size_t size) {
    return castToU8(std::string_view(data, size));
}

/*!
 * \brief u8stringをstringにキャストする
 * \since ver2.0
 */
WEBCFACE_DLL std::string_view castFromU8(std::u8string_view name);

/*!
 * \brief u8stringとstringとwstringをshared_ptrで持ち共有する
 * \since ver2.0
 *
 * 初期状態ではdataがnullptr、またはu8sのみ値を持ちsとwsは空
 *
 * decodeやdecodeWが呼ばれたときsとwsに変換後の文字列を保存する。
 * 一度保存したsやwsを別の値に書き換えることはない
 * (のでc_strなどの参照は保持される)
 *
 */
class WEBCFACE_DLL SharedString {
    struct Data {
        std::u8string u8s;
        std::string s;
        std::wstring ws;
        std::mutex m;
        explicit Data(std::u8string &&u8)
            : u8s(std::move(u8)), s(), ws(), m() {}
        explicit Data(std::u8string_view u8) : u8s(u8), s(), ws(), m() {}
        explicit Data(std::string_view s)
            : u8s(Encoding::encode(s)), s(s), ws(), m() {}
        explicit Data(std::wstring_view ws)
            : u8s(Encoding::encodeW(ws)), s(), ws(ws), m() {}
    };
    std::shared_ptr<Data> data;

  public:
    SharedString() : data() {}
    SharedString(std::nullptr_t) : data() {}
    explicit SharedString(std::u8string &&u8)
        : data(std::make_shared<Data>(std::move(u8))) {}
    explicit SharedString(std::u8string_view u8)
        : data(std::make_shared<Data>(u8)) {}
    explicit SharedString(const char8_t *u8)
        : data(std::make_shared<Data>(std::u8string_view(u8))) {}
    explicit SharedString(std::string_view s)
        : data(std::make_shared<Data>(s)) {}
    explicit SharedString(std::wstring_view ws)
        : data(std::make_shared<Data>(ws)) {}

    const std::u8string &u8String() const;
    std::u8string_view u8StringView() const;
    const std::string &decode() const;
    const std::wstring &decodeW() const;

    bool empty() const;

    bool operator==(const SharedString &other) const {
        return this->u8String() == other.u8String();
    }
    bool operator<(const SharedString &other) const {
        return this->u8String() < other.u8String();
    }

    struct Hash : std::hash<std::u8string> {
        Hash() = default;
        auto operator()(const SharedString &ss) const {
            return this->std::hash<std::u8string>::operator()(ss.u8String());
        }
    };
};

template <typename T>
using StrMap1 = std::unordered_map<SharedString, T, SharedString::Hash>;
template <typename T>
using StrMap2 = StrMap1<StrMap1<T>>;
using StrSet1 = std::unordered_set<SharedString, SharedString::Hash>;
using StrSet2 = StrMap1<StrSet1>;

} // namespace Encoding
WEBCFACE_NS_END

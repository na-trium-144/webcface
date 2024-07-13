#pragma once
#include <string>
#include <string_view>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
inline namespace encoding {
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
 * \brief u8stringとstringとwstringをshared_ptrで持ち共有する
 * \since ver2.0
 *
 * * 初期状態ではdataがnullptr、またはu8sのみ値を持ちsとwsは空
 * * decodeやdecodeWが呼ばれたときsとwsに変換後の文字列を保存する。
 * 一度保存したsやwsを別の値に書き換えることはない
 * (のでc_strなどの参照は保持される)
 * * string→utf8:
 * windowsではusingUTF8(false)の場合ANSIからutf8へエンコーディングの変換を行うが、
 * usingUTF8(true)の場合なにもせずそのままコピーする。
 * * utf-8→string: windowsでusingUTF8(false)の場合はANSIに、
 * それ以外の場合なにもせずそのままコピーする。
 *
 */
class WEBCFACE_DLL SharedString {
    struct Data {
        std::string u8s;
        std::string s;
        std::wstring ws;
        std::recursive_mutex m;
        Data(std::string_view u8s) : u8s(u8s), s(), ws(), m() {}
        Data(std::string_view u8s, std::string_view s, std::wstring_view ws)
            : u8s(u8s), s(s), ws(ws), m() {}
    };
    std::shared_ptr<Data> data;

  public:
    SharedString() : data() {}
    SharedString(std::nullptr_t) : data() {}
    explicit SharedString(std::shared_ptr<Data> &&data)
        : data(std::move(data)) {}

    static SharedString fromU8String(std::string_view u8s);
    static SharedString encode(std::string_view s);
    static SharedString encode(std::wstring_view ws,
                               std::string_view s = std::string_view());

    const std::string &u8String() const;
    std::string_view u8StringView() const;
    const std::string &decode() const;
    const std::wstring &decodeW() const;

    bool empty() const;
    bool startsWith(std::string_view str) const;
    bool startsWith(char str) const;

    bool operator==(const SharedString &other) const {
        return this->u8String() == other.u8String();
    }
    bool operator!=(const SharedString &other) const {
        return this->u8String() != other.u8String();
    }
    bool operator<(const SharedString &other) const {
        return this->u8String() < other.u8String();
    }
    bool operator<=(const SharedString &other) const {
        return this->u8String() <= other.u8String();
    }
    bool operator>(const SharedString &other) const {
        return this->u8String() > other.u8String();
    }
    bool operator>=(const SharedString &other) const {
        return this->u8String() > other.u8String();
    }

    struct Hash : std::hash<std::string> {
        Hash() = default;
        auto operator()(const SharedString &ss) const {
            return this->std::hash<std::string>::operator()(ss.u8String());
        }
    };
};

template <typename T>
using StrMap1 = std::unordered_map<SharedString, T, SharedString::Hash>;
template <typename T>
using StrMap2 = StrMap1<StrMap1<T>>;
using StrSet1 = std::unordered_set<SharedString, SharedString::Hash>;
using StrSet2 = StrMap1<StrSet1>;

} // namespace encoding
WEBCFACE_NS_END

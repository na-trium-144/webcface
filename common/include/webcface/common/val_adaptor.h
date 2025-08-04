#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <cstdint>
#include <ostream>
#include <variant>
#include "encoding.h"
#ifdef WEBCFACE_MESON
#include "webcface-config.h"
#else
#include "webcface/common/webcface-config.h"
#endif

WEBCFACE_NS_BEGIN

/*!
 * \brief 引数や戻り値の型を表すenum
 *
 */
enum class ValType {
    none_ = 0,
    string_ = 1,
    bool_ = 2,
    int_ = 3,
    float_ = 4,
    double_ = 4,
};
/*!
 * \brief TのValTypeを得る
 *
 */
template <typename T>
ValType valTypeOf() {
    if constexpr (std::is_void_v<T>) {
        return ValType::none_;
    } else if constexpr (std::is_same_v<bool, T>) {
        return ValType::bool_;
    } else if constexpr (std::is_integral_v<T>) {
        return ValType::int_;
    } else if constexpr (std::is_floating_point_v<T>) {
        return ValType::float_;
    } else {
        return ValType::string_;
    }
}

/*!
 * \brief 型名を文字列で取得
 * \since ver1.9.1
 */
inline std::string valTypeStr(ValType a) {
    switch (a) {
    case ValType::none_:
        return "none";
    case ValType::string_:
        return "string";
    case ValType::bool_:
        return "bool";
    case ValType::int_:
        return "int";
    case ValType::float_:
        return "float";
    default:
        return "unknown";
    }
}
/*!
 * \brief 型名を出力する。
 *
 */
inline std::ostream &operator<<(std::ostream &os, ValType a) {
    return os << valTypeStr(a);
}

/*!
 * \brief 数値、文字列などの値を相互変換するクラス
 *
 * 数値の場合doubleまたはint64_tで保持する
 *
 * 数値型であっても文字列のインスタンスを内部に保持し、
 * ValAdaptorが破棄されるまでの間char*やstring_viewから参照できる
 *
 * 空の状態=空文字列
 *
 */
class WEBCFACE_DLL ValAdaptor {
    /*!
     * 文字列に変換したものを保存
     * デフォルトでu8strの空文字列
     */
    mutable SharedString as_str;

    std::variant<double, std::int64_t> as_val;
    ValType type;

    enum ValVariant { DOUBLEV = 0, INT64V = 1 };

    void initStr() const;
    void initWStr() const;

  public:
    ValAdaptor();

    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(const SharedString &str);
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(const SharedString &str);

    /*!
     * ver2.10〜: std::string_view, std::wstring_view, const char*, const
     * wchar_t* を受け取るコンストラクタを String に置き換え
     *
     */
    explicit ValAdaptor(String str);
    /*!
     * ver2.10〜: std::string_view, std::wstring_view, const char*, const
     * wchar_t* を受け取るコンストラクタを String に置き換え
     *
     */
    ValAdaptor &operator=(String str);

    explicit ValAdaptor(bool value);
    ValAdaptor &operator=(bool v);

    explicit ValAdaptor(std::int64_t value);
    ValAdaptor &operator=(std::int64_t v);

    explicit ValAdaptor(double value);
    ValAdaptor &operator=(double v);

    template <typename T, typename std::enable_if_t<std::is_integral_v<T>,
                                                    std::nullptr_t> = nullptr>
    explicit ValAdaptor(T value)
        : ValAdaptor(static_cast<std::int64_t>(value)) {}
    template <typename T, typename std::enable_if_t<std::is_integral_v<T>,
                                                    std::nullptr_t> = nullptr>
    ValAdaptor &operator=(T v) {
        return *this = static_cast<std::int64_t>(v);
    }

    template <typename T, typename std::enable_if_t<std::is_floating_point_v<T>,
                                                    std::nullptr_t> = nullptr>
    explicit ValAdaptor(T value) : ValAdaptor(static_cast<double>(value)) {}
    template <typename T, typename std::enable_if_t<std::is_floating_point_v<T>,
                                                    std::nullptr_t> = nullptr>
    ValAdaptor &operator=(T v) {
        return *this = static_cast<double>(v);
    }

    ValType valType() const { return type; }

    static const ValAdaptor &emptyVal();

    /*!
     * \brief 値が空かどうか調べる
     * \since ver1.11
     */
    bool empty() const;

    /*!
     * \brief 文字列として返す
     * \since ver1.10
     *
     * std::stringのconst参照を返す。
     * 参照はこのValAdaptorが破棄されるまで有効
     *
     * \deprecated ver2.10〜
     * 内部の仕様変更により文字列のコピーが発生する可能性がある。
     * コピーなしで文字列を参照するには asStringView() または asCStr()
     * を使用すること。
     */
    [[deprecated("(ver2.10〜) use asStringView() or asCStr() instead")]]
    const std::string &asStringRef() const;
    /*!
     * \brief 文字列として返す
     * \since ver2.10
     *
     * 参照はこのValAdaptorが破棄されるまで有効
     *
     * as_strにstringが格納されていた場合はそれをそのまま返す。
     * そうでない場合(u8string, wstring, double, int64が格納されている場合)
     * はそれをstringに変換したうえでその参照を返す。
     *
     */
    std::string_view asStringView() const;
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver2.0
     * \sa asStringRef()
     * \deprecated ver2.10〜
     * 内部の仕様変更により文字列のコピーが発生する可能性がある。
     * コピーなしで文字列を参照するには asWStringView() または asWCStr()
     * を使用すること。
     */
    [[deprecated("(ver2.10〜) use asWStringView() or asWCStr() instead")]]
    const std::wstring &asWStringRef() const;
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver2.10
     *
     * 参照はこのValAdaptorが破棄されるまで有効
     *
     * as_strにwstringが格納されていた場合はそれをそのまま返す。
     * そうでない場合(u8string, string, double, int64が格納されている場合)
     * はそれをwstringに変換したうえでその参照を返す。
     */
    std::wstring_view asWStringView() const;
    /*!
     * \since ver2.0
     * \deprecated ver2.10〜
     */
    [[deprecated("(ver2.10〜) use asU8StringView() instead")]]
    const std::string &asU8StringRef() const;
    /*!
     * \since ver2.10
     */
    std::string_view asU8StringView() const;
    /*!
     * \brief 文字列として返す(コピー)
     * \since ver1.10
     */
    std::string asString() const { return std::string(asStringView()); }
    /*!
     * \brief 文字列として返す(コピー) (wstring)
     * \since ver2.0
     */
    std::wstring asWString() const { return std::wstring(asWStringView()); }
    /*!
     * \brief null終端の文字列を返す
     * \since ver2.10
     *
     * ポインタはこのValAdaptorが破棄されるまで有効
     *
     * as_strにstringが格納されていた場合はそれをそのまま返す。
     * そうでない場合(u8string, string, double, int64が格納されている場合)
     * はそれをstringに変換したうえでその参照を返す。
     */
    const char *asCStr() const;
    /*!
     * \brief null終端の文字列を返す (wstring)
     * \since ver2.0
     *
     * ポインタはこのValAdaptorが破棄されるまで有効
     *
     * as_strにwstringが格納されていた場合はそれをそのまま返す。
     * そうでない場合(u8string, string, double, int64が格納されている場合)
     * はそれをwstringに変換したうえでその参照を返す。
     */
    const wchar_t *asWCStr() const;

    /*!
     * \since ver2.10
     *
     * 以前の const std::string& を置き換え
     */
    operator std::string_view() const { return asStringView(); }
    /*!
     * \since ver2.10
     */
    operator std::wstring_view() const { return asWStringView(); }
    /*!
     * \since ver2.0
     */
    operator const char *() const { return asCStr(); }
    /*!
     * \since ver2.0
     */
    operator const wchar_t *() const { return asWCStr(); }

    /*!
     * \brief 実数として返す
     * \since ver2.0
     */
    double asDouble() const;
    /*!
     * \brief int型の整数として返す
     * \since ver2.0
     */
    int asInt() const;
    /*!
     * \brief long long型の整数として返す
     * \since ver2.0
     */
    long long asLLong() const;
    /*!
     * \brief 数値として返す
     * \since ver1.10
     *
     * as<T>(), Tはdoubleなどの実数型、intなどの整数型
     *
     * \deprecated ver2.0〜 asDouble(), asInt(), asLLong() を追加
     * さらにas<T>にはTになにを指定してもdoubleで返るというバグがある
     *
     */
    template <typename T>
    [[deprecated("use asDouble(), asInt() or asLLong() instead")]]
    double as() const {
        return static_cast<T>(asDouble());
    }
    /*!
     * \brief 数値型への変換
     */
    template <typename T,
              typename std::enable_if_t<std::is_convertible_v<double, T> &&
                                            !std::is_same_v<T, bool>,
                                        std::nullptr_t> = nullptr>
    operator T() const {
        if constexpr (std::is_floating_point_v<T>) {
            return static_cast<T>(asDouble());
        } else if constexpr (sizeof(T) > sizeof(int)) {
            return static_cast<T>(asLLong());
        } else {
            return static_cast<T>(asInt());
        }
    }

    /*!
     * \brief bool値を返す
     * \since ver1.10
     *
     * * 文字列型が入っていた場合、空文字列でなければtrueを返す
     * * 数値型が入っていた場合、0でなければtrueを返す
     *
     */
    bool asBool() const;
    /*!
     * boolへ変換
     */
    operator bool() const { return asBool(); }

    bool operator==(const ValAdaptor &other) const;
    bool operator!=(const ValAdaptor &other) const { return !(*this == other); }

    template <typename T, typename std::enable_if_t<
                              std::is_constructible_v<ValAdaptor, T> &&
                                  !std::is_same_v<ValAdaptor, T>,
                              std::nullptr_t> = nullptr>
    bool operator==(const T &other) const {
        return *this == ValAdaptor(other);
    }
    template <typename T, typename std::enable_if_t<
                              std::is_constructible_v<ValAdaptor, T> &&
                                  !std::is_same_v<ValAdaptor, T>,
                              std::nullptr_t> = nullptr>
    bool operator!=(const T &other) const {
        return *this != ValAdaptor(other);
    }
};

template <typename T,
          typename std::enable_if_t<std::is_constructible_v<ValAdaptor, T> &&
                                        !std::is_same_v<ValAdaptor, T>,
                                    std::nullptr_t> = nullptr>
bool operator==(const T &other, const ValAdaptor &val) {
    return val == ValAdaptor(other);
}
template <typename T,
          typename std::enable_if_t<std::is_constructible_v<ValAdaptor, T> &&
                                        !std::is_same_v<ValAdaptor, T>,
                                    std::nullptr_t> = nullptr>
bool operator!=(const T &other, const ValAdaptor &val) {
    return val != ValAdaptor(other);
}

inline std::ostream &operator<<(std::ostream &os, const ValAdaptor &a) {
    return os << static_cast<std::string>(a);
}

/*!
 * \brief ValAdaptorのリストから任意の型のタプルに変換する
 *
 */
template <int n = 0, typename T>
void argToTuple(const std::vector<ValAdaptor> &args, T &tuple) {
    constexpr int tuple_size = std::tuple_size<T>::value;
    if constexpr (n < tuple_size) {
        using Type = typename std::tuple_element<n, T>::type;
        std::get<n>(tuple) = static_cast<Type>(args[n]);
        argToTuple<n + 1>(args, tuple);
    }
}

namespace [[deprecated("symbols in webcface::encoding namespace are "
                       "now directly in webcface namespace")]] encoding {
using ValType = webcface::ValType;
template <typename T>
webcface::ValType valTypeOf() {
    return webcface::valTypeOf<T>();
}
inline std::string valTypeStr(webcface::ValType a) {
    return webcface::valTypeStr(a);
}
using ValAdaptor = webcface::ValAdaptor;

template <int n = 0, typename T>
void argToTuple(const std::vector<webcface::ValAdaptor> &args, T &tuple) {
    webcface::argToTuple<n>(args, tuple);
}

} // namespace encoding
WEBCFACE_NS_END

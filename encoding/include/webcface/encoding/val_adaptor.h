#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <concepts>
#include <cstdint>
#include <ostream>
#include <variant>
#include "encoding.h"
#include <webcface/common/def.h>

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
     * \since ver2.0
     */
    explicit ValAdaptor(std::u8string_view str);
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(std::u8string_view str);

    explicit ValAdaptor(std::string_view str);
    ValAdaptor &operator=(std::string_view str);
    explicit ValAdaptor(const char *str) : ValAdaptor(std::string_view(str)) {}
    ValAdaptor &operator=(const char *str) {
        return *this = std::string_view(str);
    }

    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(std::wstring_view str);
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(std::wstring_view str);
    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(const wchar_t *str)
        : ValAdaptor(std::wstring_view(str)) {}
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(const wchar_t *str) {
        return *this = std::wstring_view(str);
    }

    explicit ValAdaptor(bool value);
    ValAdaptor &operator=(bool v);

    explicit ValAdaptor(std::int64_t value);
    ValAdaptor &operator=(std::int64_t v);

    explicit ValAdaptor(double value);
    ValAdaptor &operator=(double v);

    template <typename T>
        requires std::integral<T>
    explicit ValAdaptor(T value)
        : ValAdaptor(static_cast<std::int64_t>(value)) {}
    template <typename T>
        requires std::integral<T>
    ValAdaptor &operator=(T v) {
        return *this = static_cast<std::int64_t>(v);
    }

    template <typename T>
        requires std::floating_point<T>
    explicit ValAdaptor(T value) : ValAdaptor(static_cast<double>(value)) {}
    template <typename T>
        requires std::floating_point<T>
    ValAdaptor &operator=(T v) {
        return *this = static_cast<double>(v);
    }

    ValType valType() const { return type; }

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
     * as_strにstringが格納されていた場合はそれをそのまま返す。
     * そうでない場合(u8string, wstring, double, int64が格納されている場合)
     * はそれをstringに変換したうえでその参照を返す。
     *
     */
    const std::string &asStringRef() const;
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver2.0
     * \sa asStringRef()
     */
    const std::wstring &asWStringRef() const;
    /*!
     * \since ver2.0
     */
    const std::u8string &asU8StringRef() const;

    /*!
     * \brief 文字列として返す(コピー)
     * \since ver1.10
     */
    std::string asString() const { return asStringRef(); }
    /*!
     * \brief 文字列として返す(コピー) (wstring)
     * \since ver2.0
     */
    std::wstring asWString() const { return asWStringRef(); }

    /*!
     * ver1.10〜: const参照
     */
    operator const std::string &() const { return asStringRef(); }
    /*!
     * \since ver2.0
     */
    operator const std::wstring &() const { return asWStringRef(); }
    /*!
     * \since ver2.0
     */
    operator const char *() const { return asStringRef().c_str(); }
    /*!
     * \since ver2.0
     */
    operator const wchar_t *() const { return asWStringRef().c_str(); }

    /*!
     * \brief string_viewなどへの変換
     */
    template <typename T>
        requires std::convertible_to<std::string, T>
    operator T() const {
        return static_cast<T>(asStringRef());
    }
    /*!
     * \brief wstring_viewなどへの変換
     * \since ver2.0
     */
    template <typename T>
        requires(std::convertible_to<std::wstring, T> &&
                 !std::convertible_to<std::string, T>)
    operator T() const {
        return static_cast<T>(asWStringRef());
    }

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
     *
     */
    template <typename T>
        requires(std::convertible_to<double, T> && !std::same_as<T, bool>)
    [[deprecated("use asDouble(), asInt() or asLLong() instead")]] double as()
        const {
        return static_cast<T>(asDouble());
    }
    /*!
     * \brief 数値型への変換
     */
    template <typename T>
        requires(std::convertible_to<double, T> && !std::same_as<T, bool>)
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

    template <typename T>
        requires(std::constructible_from<ValAdaptor, T> &&
                 !std::same_as<ValAdaptor, T>) bool
    operator==(const T &other) const {
        return *this == ValAdaptor(other);
    }
};

template <typename T>
    requires(std::constructible_from<ValAdaptor, T> &&
             !std::same_as<ValAdaptor, T>) bool
operator==(const T &other, const ValAdaptor &val) {
    return val == ValAdaptor(other);
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
WEBCFACE_NS_END

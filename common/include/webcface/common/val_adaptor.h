#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <cstdint>
#include <ostream>
#include "encoding.h"
#include "c_val_adaptor.h"

WEBCFACE_NS_BEGIN

/*!
 * \brief 引数や戻り値の型を表すenum
 *
 */
enum class ValType {
    none_ = wcfValType::WCF_VAL_NONE,
    string_ = wcfValType::WCF_VAL_STRING,
    bool_ = wcfValType::WCF_VAL_BOOL,
    int_ = wcfValType::WCF_VAL_INT,
    float_ = wcfValType::WCF_VAL_DOUBLE,
    double_ = wcfValType::WCF_VAL_DOUBLE,
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
class ValAdaptor {
    /*!
     * 文字列に変換したものを保存
     * デフォルトでu8strの空文字列
     */
    mutable SharedString as_str;
    double as_double = 0;
    std::int64_t as_int = 0;
    ValType type = ValType::none_;

    WEBCFACE_DLL void initU8String() const;
    WEBCFACE_DLL void initString() const;
    WEBCFACE_DLL void initWString() const;

  public:
    ValAdaptor() = default;

    /*!
     * \since ver2.0
     */
    WEBCFACE_DLL explicit ValAdaptor(const SharedString &str);
    /*!
     * \since ver2.0
     */
    WEBCFACE_DLL ValAdaptor &operator=(const SharedString &str);

    explicit ValAdaptor(std::string_view str)
        : ValAdaptor(SharedString::encode(str)) {}
    ValAdaptor &operator=(std::string_view str) {
        return *this = SharedString::encode(str);
    }
    explicit ValAdaptor(const char *str) : ValAdaptor(std::string_view(str)) {}
    ValAdaptor &operator=(const char *str) {
        return *this = std::string_view(str);
    }

    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(std::wstring_view str)
        : ValAdaptor(SharedString::encode(str)) {}
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(std::wstring_view str) {
        return *this = SharedString::encode(str);
    }
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

    WEBCFACE_DLL explicit ValAdaptor(bool value);
    WEBCFACE_DLL ValAdaptor &operator=(bool v);

    WEBCFACE_DLL explicit ValAdaptor(std::int64_t value);
    WEBCFACE_DLL ValAdaptor &operator=(std::int64_t v);

    WEBCFACE_DLL explicit ValAdaptor(double value);
    WEBCFACE_DLL ValAdaptor &operator=(double v);

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

    static WEBCFACE_DLL const ValAdaptor &WEBCFACE_CALL emptyVal();

    /*!
     * \brief 値が空かどうか調べる
     * \since ver1.11
     */
    WEBCFACE_DLL bool empty() const;

    /*!
     * \brief 文字列として返す
     * \since ver1.10
     *
     * * <del>std::stringのconst参照を返す。</del>
     * * (ver3.0〜) string_viewを返す。
     * * 参照はこのValAdaptorが破棄されるまで有効
     * * as_strにstringが格納されていた場合はそれをそのまま返す。
     * そうでない場合(u8string, wstring, double, int64が格納されている場合)
     * はそれをstringに変換したうえでその参照を返す。
     *
     */
    std::string_view asStringRef() const {
        initString();
        return as_str.u8String();
    }
    /*!
     * \since ver3.0
     */
    const char *asCStr() const { return asStringRef().data(); }
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver2.0
     * \sa asStringRef()
     */
    std::wstring_view asWStringRef() const {
        initWString();
        return as_str.decodeW();
    }
    /*!
     * \since ver3.0
     */
    const wchar_t *asWCStr() const { return asWStringRef().data(); }
    /*!
     * \since ver2.0
     */
    std::string_view asU8StringRef() const {
        initU8String();
        return as_str.u8String();
    }
    /*!
     * \since ver3.0
     */
    const char *asU8CStr() const { return asU8StringRef().data(); }

    /*!
     * \brief 文字列として返す(コピー)
     * \since ver1.10
     */
    std::string asString() const { return std::string(asStringRef()); }
    /*!
     * \brief 文字列として返す(コピー) (wstring)
     * \since ver2.0
     */
    std::wstring asWString() const { return std::wstring(asWStringRef()); }

    /*!
     * * (ver1.10〜) <del>const参照に変更</del>
     * * (ver3.0〜) string_viewに変更
     */
    operator std::string_view() const { return asStringRef(); }
    /*!
     * \since ver2.0
     *
     * * (ver3.0〜) wstring_viewに変更
     */
    operator std::wstring_view() const { return asWStringRef(); }
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
    WEBCFACE_DLL double asDouble() const;
    /*!
     * \brief int型の整数として返す
     * \since ver2.0
     */
    WEBCFACE_DLL int asInt() const;
    /*!
     * \brief long long型の整数として返す
     * \since ver2.0
     */
    WEBCFACE_DLL long long asLLong() const;
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
    [[deprecated("use asDouble(), asInt() or asLLong() instead")]] double
    as() const {
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
    WEBCFACE_DLL bool asBool() const;
    /*!
     * boolへ変換
     */
    operator bool() const { return asBool(); }

    WEBCFACE_DLL bool operator==(const ValAdaptor &other) const;
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

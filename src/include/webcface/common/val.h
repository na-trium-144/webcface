#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <concepts>
#include <stdexcept>
#include <cstdint>
#include <ostream>
#include <variant>
#include <optional>
#include "../c_wcf/def_types.h"
#include "webcface/encoding.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
inline namespace Common {
//! 引数や戻り値の型を表すenum
enum class ValType {
    none_ = 0,
    string_ = 1,
    bool_ = 2,
    int_ = 3,
    float_ = 4,
    double_ = 4,
};
//! TのValTypeを得る
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

    std::variant<double, std::int64_t> as_val;
    ValType type;

    enum StrVariant { U8STR = 0, STR = 1, WSTR = 2 };
    enum ValVariant { DOUBLEV = 0, INT64V = 1 };

  public:
    ValAdaptor() : type(ValType::none_) {}

    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(const SharedString &str)
        : as_str(str), type(ValType::string_) {}
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(const SharedString &str) {
        as_str = str;
        type = ValType::string_;
        return *this;
    }
    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(std::u8string_view str)
        : as_str(str), type(ValType::string_) {}
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(std::u8string_view str) {
        as_str = SharedString(str);
        type = ValType::string_;
        return *this;
    }

    explicit ValAdaptor(std::string_view str)
        : as_str(str), type(ValType::string_) {}
    ValAdaptor &operator=(std::string_view str) {
        as_str = SharedString(str);
        type = ValType::string_;
        return *this;
    }
    explicit ValAdaptor(const char *str)
        : as_str(str), type(ValType::string_) {}
    ValAdaptor &operator=(const char *str) {
        as_str = SharedString(str);
        type = ValType::string_;
        return *this;
    }

    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(std::wstring_view str)
        : as_str(str), type(ValType::string_) {}
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(std::wstring_view str) {
        as_str = SharedString(str);
        type = ValType::string_;
        return *this;
    }
    /*!
     * \since ver2.0
     */
    explicit ValAdaptor(const wchar_t *str)
        : as_str(str), type(ValType::string_) {}
    /*!
     * \since ver2.0
     */
    ValAdaptor &operator=(const wchar_t *str) {
        as_str = SharedString(str);
        type = ValType::string_;
        return *this;
    }

    explicit ValAdaptor(bool value)
        : as_val(static_cast<std::int64_t>(value)), type(ValType::bool_) {}
    ValAdaptor &operator=(bool v) {
        as_val.emplace<INT64V>(v);
        type = ValType::bool_;
        return *this;
    }

    template <typename T>
        requires std::integral<T>
    explicit ValAdaptor(T value)
        : as_val(static_cast<std::int64_t>(value)), type(ValType::int_) {}
    template <typename T>
        requires std::integral<T>
    ValAdaptor &operator=(T v) {
        as_val.emplace<INT64V>(v);
        type = ValType::int_;
        return *this;
    }

    template <typename T>
        requires std::floating_point<T>
    explicit ValAdaptor(T value)
        : as_val(static_cast<double>(value)), type(ValType::float_) {}
    template <typename T>
        requires std::floating_point<T>
    ValAdaptor &operator=(T v) {
        as_val.emplace<DOUBLEV>(v);
        type = ValType::float_;
        return *this;
    }

    /*!
     * \brief wcfMultiValから変換
     *
     * valのint, double, strのいずれか1つに値をセットして渡すと、
     * データ型を判別する
     *
     * as_strの文字列はコピーして保持する
     *
     */
    explicit ValAdaptor(const wcfMultiVal &val) {
        if (val.as_str != nullptr) {
            this->as_str = SharedString(val.as_str);
            type = ValType::string_;
        } else if (val.as_double != 0) {
            this->as_val.emplace<DOUBLEV>(val.as_double);
            type = ValType::float_;
        } else {
            this->as_val.emplace<INT64V>(val.as_int);
            type = ValType::int_;
        }
    }
    /*!
     * \brief wcfMultiValWから変換
     * \since ver2.0
     */
    explicit ValAdaptor(const wcfMultiValW &val) {
        if (val.as_str != nullptr) {
            this->as_str = SharedString(val.as_str);
            type = ValType::string_;
        } else if (val.as_double != 0) {
            this->as_val.emplace<DOUBLEV>(val.as_double);
            type = ValType::float_;
        } else {
            this->as_val.emplace<INT64V>(val.as_int);
            type = ValType::int_;
        }
    }


    ValType valType() const { return type; }

    /*!
     * \brief 値が空かどうか調べる
     * \since ver1.11
     */
    bool empty() const {
        if (type == ValType::none_ || type == ValType::string_) {
            return as_str.empty();
        } else {
            return false;
        }
    }


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
    const std::string &asStringRef() const {
        if (as_str.empty() && valType() != ValType::none_ &&
            valType() != ValType::string_) {
            if (as_val.index() == DOUBLEV) {
                as_str =
                    SharedString(std::to_string(std::get<DOUBLEV>(as_val)));
            } else {
                as_str = SharedString(std::to_string(std::get<INT64V>(as_val)));
            }
        }
        return as_str.decode();
    }
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver2.0
     * \sa asStringRef()
     */
    const std::wstring &asWStringRef() const {
        if (as_str.empty() && valType() != ValType::none_ &&
            valType() != ValType::string_) {
            if (as_val.index() == DOUBLEV) {
                as_str =
                    SharedString(std::to_wstring(std::get<DOUBLEV>(as_val)));
            } else {
                as_str =
                    SharedString(std::to_wstring(std::get<INT64V>(as_val)));
            }
        }
        return as_str.decodeW();
    }
    /*!
     * \since ver2.0
     */
    const std::u8string &asU8StringRef() const {
        if (as_str.empty() && valType() != ValType::none_ &&
            valType() != ValType::string_) {
            if (as_val.index() == DOUBLEV) {
                as_str =
                    SharedString(std::to_string(std::get<DOUBLEV>(as_val)));
            } else {
                as_str = SharedString(std::to_string(std::get<INT64V>(as_val)));
            }
        }
        return as_str.u8String();
    }

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
     * \brief 数値として返す
     * \since ver1.10
     *
     * as<T>(), Tはdoubleなどの実数型、intなどの整数型
     *
     */
    template <typename T>
        requires(std::convertible_to<double, T> && !std::same_as<T, bool>)
    double as() const {
        if (type == ValType::string_) {
            try {
                return std::stod(asStringRef());
            } catch (...) {
                return 0;
            }
        } else {
            switch (as_val.index()) {
            case DOUBLEV:
                return std::get<DOUBLEV>(as_val);
            default:
                return static_cast<double>(std::get<INT64V>(as_val));
            }
        }
    }
    /*!
     * \brief 数値型への変換
     */
    template <typename T>
        requires(std::convertible_to<double, T> && !std::same_as<T, bool>)
    operator T() const {
        return as<T>();
    }

    /*!
     * \brief bool値を返す
     * \since ver1.10
     *
     * * 文字列型が入っていた場合、空文字列でなければtrueを返す
     * * 数値型が入っていた場合、0でなければtrueを返す
     *
     */
    bool asBool() const {
        if (type == ValType::string_) {
            return !empty();
        } else {
            switch (as_val.index()) {
            case DOUBLEV:
                return std::get<DOUBLEV>(as_val) != 0;
            default:
                return std::get<INT64V>(as_val) != 0;
            }
        }
    }
    /*!
     * boolへ変換
     */
    operator bool() const { return asBool(); }


    bool operator==(const ValAdaptor &other) const {
        if (type == ValType::double_ || other.type == ValType::double_) {
            return this->as<double>() == other.as<double>();
        } else if (type == ValType::int_ || other.type == ValType::int_) {
            return this->as<std::int64_t>() == other.as<std::int64_t>();
        } else if (type == ValType::bool_ || other.type == ValType::bool_) {
            return this->asBool() == other.asBool();
        } else {
            return this->asU8StringRef() == other.asU8StringRef();
        }
    }

    template <typename T>
        requires(std::constructible_from<ValAdaptor, T> &&
                 !std::same_as<ValAdaptor, T>)
    bool operator==(const T &other) const {
        return *this == ValAdaptor(other);
    }
};

template <typename T>
    requires(std::constructible_from<ValAdaptor, T> &&
             !std::same_as<ValAdaptor, T>)
bool operator==(const T &other, const ValAdaptor &val) {
    return val == ValAdaptor(other);
}

inline std::ostream &operator<<(std::ostream &os, const ValAdaptor &a) {
    return os << static_cast<std::string>(a);
}

//! ValAdaptorのリストから任意の型のタプルに変換する
template <int n = 0, typename T>
void argToTuple(const std::vector<ValAdaptor> &args, T &tuple) {
    constexpr int tuple_size = std::tuple_size<T>::value;
    if constexpr (n < tuple_size) {
        using Type = typename std::tuple_element<n, T>::type;
        std::get<n>(tuple) = static_cast<Type>(args[n]);
        argToTuple<n + 1>(args, tuple);
    }
}
} // namespace Common
WEBCFACE_NS_END

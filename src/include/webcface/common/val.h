#pragma once
#include <__fwd/string_view.h>
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
    mutable std::variant<std::u8string, std::string, std::wstring> as_str;
    std::variant<double, std::int64_t> as_val;
    ValType type;

    enum StrVariant { U8STR = 0, STR = 1, WSTR = 2 };
    enum ValVariant { DOUBLEV = 0, INT64V = 1 };

  public:
    ValAdaptor() : type(ValType::none_) {}

    explicit ValAdaptor(std::u8string_view str)
        : as_str(std::u8string(str)), type(ValType::string_) {}
    explicit ValAdaptor(std::string_view str)
        : as_str(std::string(str)), type(ValType::string_) {}
    explicit ValAdaptor(std::wstring_view str)
        : as_str(std::wstring(str)), type(ValType::string_) {}
    explicit ValAdaptor(bool value)
        : as_val(static_cast<std::int64_t>(value)), type(ValType::bool_) {}
    template <typename T>
        requires std::integral<T>
    explicit ValAdaptor(T value)
        : as_val(static_cast<std::int64_t>(value)), type(ValType::int_) {}
    template <typename T>
        requires std::floating_point<T>
    explicit ValAdaptor(T value)
        : as_val(static_cast<double>(value)), type(ValType::float_) {}

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
            this->as_str.emplace<STR>(val.as_str);
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
            switch (as_str.index()) {
            case U8STR:
                return std::get<U8STR>(as_str).empty();
            case STR:
                return std::get<STR>(as_str).empty();
            default:
                return std::get<WSTR>(as_str).empty();
            }
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
        if (as_str.index() != STR) {
            std::string str;
            if (valType() == ValType::none_) {
                // empty
            } else if (valType() == ValType::string_) {
                if (as_str.index() == U8STR) [[likely]] {
                    str = Encoding::decode(std::get<U8STR>(as_str));
                } else if (as_str.index() == WSTR) {
                    str = Encoding::decode(
                        Encoding::encodeW(std::get<WSTR>(as_str)));
                }
            } else {
                if (as_val.index() == DOUBLEV) {
                    str = std::to_string(std::get<DOUBLEV>(as_val));
                } else {
                    str = std::to_string(std::get<INT64V>(as_val));
                }
            }
            as_str.emplace<STR>(std::move(str));
        }
        return std::get<STR>(as_str);
    }
    /*!
     * \brief 文字列として返す (wstring)
     * \since ver1.12
     * \sa asStringRef()
     */
    const std::wstring &asWStringRef() const {
        if (as_str.index() != WSTR) {
            std::wstring str;
            if (valType() == ValType::none_) {
                // empty
            } else if (valType() == ValType::string_) {
                if (as_str.index() == U8STR) [[likely]] {
                    str = Encoding::decodeW(std::get<U8STR>(as_str));
                } else if (as_str.index() == STR) {
                    str = Encoding::decodeW(
                        Encoding::encode(std::get<STR>(as_str)));
                }
            } else {
                if (as_val.index() == DOUBLEV) {
                    str = std::to_wstring(std::get<DOUBLEV>(as_val));
                } else {
                    str = std::to_wstring(std::get<INT64V>(as_val));
                }
            }
            as_str.emplace<WSTR>(std::move(str));
        }
        return std::get<WSTR>(as_str);
    }
    /*!
     * \brief 文字列として返す(コピー)
     * \since ver1.10
     */
    std::string asString() const { return asStringRef(); }
    /*!
     * \brief 文字列として返す(コピー) (wstring)
     * \since ver1.12
     */
    std::wstring asWString() const { return asWStringRef(); }
    operator const std::string &() const { return asStringRef(); }
    operator const std::wstring &() const { return asWStringRef(); }
    template <typename T>
        requires std::convertible_to<std::string, T>
    operator T() const {
        return static_cast<T>(asStringRef());
    }
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
                switch (as_str.index()) {
                case U8STR:
                case STR:
                    return std::stod(asStringRef());
                default:
                    return std::stod(asWStringRef());
                }
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
    operator bool() const { return asBool(); }

    ValAdaptor &operator=(bool v) {
        as_val.emplace<INT64V>(v);
        type = ValType::bool_;
        return *this;
    }
    template <typename T>
        requires std::integral<T>
    ValAdaptor &operator=(T v) {
        as_val.emplace<INT64V>(v);
        type = ValType::int_;
        return *this;
    }
    template <typename T>
        requires std::floating_point<T>
    ValAdaptor &operator=(T v) {
        as_val.emplace<DOUBLEV>(v);
        type = ValType::float_;
        return *this;
    }
    ValAdaptor &operator=(std::string_view v) {
        as_str.emplace<STR>(v);
        type = ValType::string_;
        return *this;
    }
    ValAdaptor &operator=(std::wstring_view v) {
        as_str.emplace<WSTR>(v);
        type = ValType::string_;
        return *this;
    }

    bool operator==(const ValAdaptor &other) const {
        if (type == ValType::double_ || other.type == ValType::double_) {
            return this->as<double>() == other.as<double>();
        } else if (type == ValType::int_ || other.type == ValType::int_) {
            return this->as<std::int64_t>() == other.as<std::int64_t>();
        } else if (type == ValType::bool_ || other.type == ValType::bool_) {
            return this->asBool() == other.asBool();
        } else {
            if (this->as_str.index() == other.as_str.index()) {
                return this->as_str == other.as_str;
            } else if (this->as_str.index() == WSTR ||
                       other.as_str.index() == WSTR) {
                return this->asWStringRef() == other.asWStringRef();
            } else {
                return this->asStringRef() == other.asStringRef();
            }
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

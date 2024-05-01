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
#include "def.h"

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
    mutable std::optional<std::string> as_str;
    std::variant<double, std::int64_t> as_val;
    ValType type;

  public:
    ValAdaptor() : type(ValType::none_) {}

    explicit ValAdaptor(const std::string &value)
        : as_str(value), type(ValType::string_) {}
    explicit ValAdaptor(const char *value)
        : as_str(value), type(ValType::string_) {}
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
            this->as_str.emplace(val.as_str);
            type = ValType::string_;
        } else if (val.as_double != 0) {
            this->as_val.emplace<0>(val.as_double);
            type = ValType::float_;
        } else {
            this->as_val.emplace<1>(val.as_int);
            type = ValType::int_;
        }
    }

    ValType valType() const { return type; }

    /*!
     * \brief 値が空かどうか調べる
     * \since ver1.11
     */
    bool empty() const {
        return (type == ValType::none_ || type == ValType::string_) &&
               (!as_str || as_str->empty());
    }

    /*!
     * \brief 文字列として返す
     * \since ver1.10
     *
     * std::stringのconst参照を返す。
     * 参照はこのValAdaptorが破棄されるまで有効
     *
     */
    const std::string &asStringRef() const {
        if (!as_str) {
            if (valType() == ValType::none_) {
                as_str.emplace("");
            } else {
                switch (as_val.index()) {
                case 0:
                    as_str.emplace(std::to_string(std::get<0>(as_val)));
                    break;
                default:
                    as_str.emplace(std::to_string(std::get<1>(as_val)));
                    break;
                }
            }
        }
        return *as_str;
    }
    /*!
     * \brief 文字列として返す(コピー)
     * \since ver1.10
     */
    std::string asString() const { return asStringRef(); }
    operator const std::string &() const { return asStringRef(); }
    template <typename T>
        requires std::convertible_to<std::string, T>
    operator T() const {
        return static_cast<T>(asStringRef());
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
            if (as_str) {
                return std::atof(as_str->c_str());
            } else {
                return 0;
            }
        } else {
            switch (as_val.index()) {
            case 0:
                return std::get<0>(as_val);
            default:
                return std::get<1>(as_val);
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
            if (as_str) {
                return !as_str->empty();
            } else {
                return false;
            }
        } else {
            switch (as_val.index()) {
            case 0:
                return std::get<0>(as_val) != 0;
            default:
                return std::get<1>(as_val) != 0;
            }
        }
    }
    operator bool() const { return asBool(); }

    ValAdaptor &operator=(bool v) {
        as_val.emplace<1>(v);
        type = ValType::bool_;
        return *this;
    }
    template <typename T>
        requires std::integral<T>
    ValAdaptor &operator=(T v) {
        as_val.emplace<1>(v);
        type = ValType::int_;
        return *this;
    }
    template <typename T>
        requires std::floating_point<T>
    ValAdaptor &operator=(T v) {
        as_val.emplace<0>(v);
        type = ValType::float_;
        return *this;
    }
    ValAdaptor &operator=(const std::string &v) {
        as_str.emplace(v);
        type = ValType::string_;
        return *this;
    }
    ValAdaptor &operator=(const char *v) {
        as_str.emplace(v);
        type = ValType::string_;
        return *this;
    }

    bool operator==(const ValAdaptor &other) const {
        if (type == ValType::string_ || other.type == ValType::string_) {
            return this->asStringRef() == other.asStringRef();
        } else if (type == ValType::double_ || other.type == ValType::double_) {
            return this->as<double>() == other.as<double>();
        } else if (type == ValType::int_ || other.type == ValType::int_) {
            return this->as<std::int64_t>() == other.as<std::int64_t>();
        } else if (type == ValType::bool_ || other.type == ValType::bool_) {
            return this->asBool() == other.asBool();
        } else {
            return this->asStringRef() == other.asStringRef();
        }
    }
    bool operator!=(const ValAdaptor &other) const { return !(*this == other); }

    template <typename T>
        requires(std::constructible_from<ValAdaptor, T> &&
                 !std::same_as<ValAdaptor, T>)
    bool operator==(const T &other) const {
        return *this == ValAdaptor(other);
    }
    template <typename T>
        requires(std::constructible_from<ValAdaptor, T> &&
                 !std::same_as<ValAdaptor, T>)
    bool operator!=(const T &other) const {
        return !(*this == other);
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

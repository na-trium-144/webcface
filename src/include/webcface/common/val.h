#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <concepts>
#include <stdexcept>
#include <cstdint>
#include <ostream>
#include "../c_wcf/def_types.h"
#include "def.h"

namespace WEBCFACE_NS {
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
 * Funcの引数、戻り値などに使う
 *
 */
class ValAdaptor {
    std::string value;
    ValType type;

  public:
    ValAdaptor() : value(""), type(ValType::none_) {}

    // cast from run()
    explicit ValAdaptor(const std::string &value)
        : value(value), type(ValType::string_) {}
    explicit ValAdaptor(const std::string &value, ValType type)
        : value(value), type(type) {}
    explicit ValAdaptor(const char *value)
        : value(value), type(ValType::string_) {}
    explicit ValAdaptor(bool value)
        : value(std::to_string(value)), type(ValType::bool_) {}
    template <typename T>
        requires std::integral<T>
    explicit ValAdaptor(T value)
        : value(std::to_string(value)), type(ValType::int_) {}
    template <typename T>
        requires std::floating_point<T>
    explicit ValAdaptor(T value)
        : value(std::to_string(value)), type(ValType::float_) {}

    /*!
     * \brief wcfMultiValから変換
     *
     * valのint, double, strのいずれか1つに値をセットして渡すと、
     * データ型を判別する
     *
     * as_strの文字列はコピーして保持する
     *
     */
    ValAdaptor(const wcfMultiVal &val) {
        if (val.as_str != nullptr) {
            value = val.as_str;
            type = ValType::string_;
        } else if (val.as_double != 0) {
            value = std::to_string(val.as_double);
            type = ValType::float_;
        } else {
            value = std::to_string(val.as_int);
            type = ValType::int_;
        }
    }

    ValType valType() const { return type; }

    // cast to function
    operator const std::string &() const { return value; }
    operator double() const { return std::atof(value.c_str()); }
    operator bool() const {
        if (type == ValType::string_) {
            return !value.empty();
        } else {
            return operator double() != 0;
        }
    }
    template <typename T>
        requires std::convertible_to<double, T>
    operator T() const {
        return static_cast<T>(operator double());
    }
    template <typename T>
        requires std::convertible_to<std::string, T>
    operator T() const {
        return static_cast<T>(operator const std::string &());
    }

    // cast from msgpack
    ValAdaptor &operator=(bool v) {
        value = std::to_string(v);
        type = ValType::bool_;
        return *this;
    }
    template <typename T>
        requires std::integral<T>
    ValAdaptor &operator=(T v) {
        value = std::to_string(v);
        type = ValType::int_;
        return *this;
    }
    template <typename T>
        requires std::floating_point<T>
    ValAdaptor &operator=(T v) {
        value = std::to_string(v);
        type = ValType::float_;
        return *this;
    }
    ValAdaptor &operator=(const std::string &v) {
        value = v;
        type = ValType::string_;
        return *this;
    }
    ValAdaptor &operator=(const char *v) {
        value = v;
        type = ValType::string_;
        return *this;
    }

    bool operator==(const ValAdaptor &other) const {
        if (type == ValType::string_ || other.type == ValType::string_) {
            return value == other.value;
        } else if (type == ValType::double_ || other.type == ValType::double_) {
            return static_cast<double>(*this) == static_cast<double>(other);
        } else if (type == ValType::int_ || other.type == ValType::int_) {
            return static_cast<int>(*this) == static_cast<int>(other);
        } else if (type == ValType::bool_ || other.type == ValType::bool_) {
            return static_cast<int>(*this) == static_cast<int>(other);
        } else {
            return value == other.value;
        }
    }
    bool operator!=(const ValAdaptor &other) const { return !(*this == other); }

    bool operator==(const char *other) const { return value == other; }
    bool operator!=(const char *other) const { return value != other; }
};

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
} // namespace WEBCFACE_NS

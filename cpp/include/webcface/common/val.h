#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <concepts>
#include <stdexcept>
#include <cstdint>
#include <ostream>

namespace WebCFace {
//! WebCFace::Commonはserverとclientで共通のheader-onlyなクラス
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
//! 型名を出力する
inline std::ostream &operator<<(std::ostream &os, ValType a) {
    switch (a) {
    case ValType::none_:
        return os << "none";
    case ValType::string_:
        return os << "string";
    case ValType::bool_:
        return os << "bool";
    case ValType::int_:
        return os << "int";
    case ValType::float_:
        return os << "float";
    default:
        return os << "unknown";
    }
}

//! 数値、文字列などの値を相互変換するクラス
//! Funcの引数、戻り値などに使う
class ValAdaptor {
    std::string value;
    ValType type;

  public:
    ValAdaptor() : value(""), type(ValType::none_) {}

    // cast from run()
    ValAdaptor(const std::string &value)
        : value(value), type(ValType::string_) {}
    ValAdaptor(const char *value) : value(value), type(ValType::string_) {}
    ValAdaptor(bool value)
        : value(std::to_string(value)), type(ValType::bool_) {}
    template <typename T>
        requires std::integral<T>
    ValAdaptor(T value) : value(std::to_string(value)), type(ValType::int_) {}
    template <typename T>
        requires std::floating_point<T>
    ValAdaptor(T value) : value(std::to_string(value)), type(ValType::float_) {}

    ValType valType() const { return type; }

    // cast to function
    operator const std::string &() const { return value; }
    operator double() const {
        try {
            return std::stod(value);
        } catch (...) {
            return 0;
        }
    }
    operator bool() const{
        return value == std::to_string(true);
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
};

// inline std::ostream &operator<<(std::ostream &os, const ValAdaptor &a) {
//     return os << static_cast<std::string>(a) << "(type=" << a.valType() << ")";
// }

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
} // namespace WebCFace
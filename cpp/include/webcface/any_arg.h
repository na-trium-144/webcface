#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <concepts>
#include <stdexcept>
#include <cstdint>
#include <ostream>

namespace WebCFace {
enum class AbstArgType {
    none_,
    string_,
    bool_,
    int_,
    float_,
};
inline std::ostream &operator<<(std::ostream &os, AbstArgType a) {
    switch (a) {
    case AbstArgType::none_:
        return os << "none";
    case AbstArgType::string_:
        return os << "string";
    case AbstArgType::bool_:
        return os << "bool";
    case AbstArgType::int_:
        return os << "int";
    case AbstArgType::float_:
        return os << "float";
    default:
        return os << "unknown";
    }
}

class AnyArg {
    std::string value;
    AbstArgType type;

  public:
    AnyArg() : value(""), type(AbstArgType::none_) {}

    // cast from run()
    explicit AnyArg(const std::string &value)
        : value(value), type(AbstArgType::string_) {}
    explicit AnyArg(const char *value)
        : value(value), type(AbstArgType::string_) {}
    explicit AnyArg(bool value)
        : value(std::to_string(value)), type(AbstArgType::bool_) {}
    template <typename T>
        requires std::integral<T>
    explicit AnyArg(T value)
        : value(std::to_string(value)), type(AbstArgType::int_) {}
    template <typename T>
        requires std::floating_point<T>
    explicit AnyArg(T value)
        : value(std::to_string(value)), type(AbstArgType::float_) {}

    AbstArgType argType() const { return type; }

    // cast to function
    operator const std::string &() const { return value; }
    operator double() const {
        try {
            return std::stod(value);
        } catch (...) {
            return 0;
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
    AnyArg &operator=(bool v) {
        value = std::to_string(v);
        type = AbstArgType::bool_;
        return *this;
    }
    template <typename T>
        requires std::integral<T>
    AnyArg &operator=(T v) {
        value = std::to_string(v);
        type = AbstArgType::int_;
        return *this;
    }
    template <typename T>
        requires std::floating_point<T>
    AnyArg &operator=(T v) {
        value = std::to_string(v);
        type = AbstArgType::float_;
        return *this;
    }
    AnyArg &operator=(const std::string &v) {
        value = v;
        type = AbstArgType::string_;
        return *this;
    }
};

inline std::ostream &operator<<(std::ostream &os, const AnyArg &a) {
    return os << static_cast<std::string>(a) << "(type=" << a.argType() << ")";
}

template <int n = 0, typename T>
void argToTuple(const std::vector<AnyArg> &args, T &tuple) {
    constexpr int tuple_size = std::tuple_size<T>::value;
    if constexpr (n < tuple_size) {
        using Type = typename std::tuple_element<n, T>::type;
        if (args.size() <= n) {
            throw std::invalid_argument(
                "requires " + std::to_string(tuple_size) + " arguments, got " +
                std::to_string(args.size()));
        }
        std::get<n>(tuple) = static_cast<Type>(args.at(n));
        argToTuple<n + 1>(args, tuple);
    }
}
} // namespace WebCFace
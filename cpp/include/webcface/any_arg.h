#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <concepts>
#include <stdexcept>

namespace WebCFace {
class AnyArg {
    std::string value;

  public:
    AnyArg() : value("") {}
    explicit AnyArg(const std::string &value) : value(value) {}
    explicit AnyArg(double value) : value(std::to_string(value)) {}
    operator const std::string &() { return value; }
    operator double() {
        try {
            return std::stod(value);
        } catch (...) {
            return 0;
        }
    }
    template <typename T>
        requires std::convertible_to<double, T>
    operator T() {
        return static_cast<T>(operator double());
    }
    template <typename T>
        requires std::convertible_to<std::string, T>
    operator T() {
        return static_cast<T>(operator const std::string &());
    }
};
template <int n = 0, typename T>
void argToTuple(std::vector<AnyArg> &args, T &tuple) {
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
template <int n = 0, typename T>
void tupleToArg(std::vector<AnyArg> &args, const T &tuple) {
    constexpr int tuple_size = std::tuple_size<T>::value;
    if constexpr (n < tuple_size) {
        args.at(n) = static_cast<AnyArg>(std::get<n>(tuple));
        tupleToArg<n + 1>(args, tuple);
    }
}
} // namespace WebCFace
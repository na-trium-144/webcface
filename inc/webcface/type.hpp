#pragma once
#include <concepts>
#include <string>
#include <memory>
#include <type_traits>

namespace WebCFace
{
// 絶対に型名を誤字しないために型を表すクラス
struct ValueType
{
    enum class Repr { unknown_, bool_, int_, float_, string_, vector_ };
    Repr repr;
    std::string getReprStr() const
    {
        switch (repr) {
        case Repr::unknown_:
            return "unknown";
        case Repr::bool_:
            return "bool";
        case Repr::int_:
            return "int";
        case Repr::float_:
            return "float";
        case Repr::string_:
            return "string";
        case Repr::vector_:
            return "vector";
        }
    }
    std::shared_ptr<ValueType> child;

    ValueType() : repr(Repr::unknown_) {}
    ValueType(const ValueType& v) : repr(v.repr), child(v.child) {}
    explicit ValueType(Repr repr) : repr(repr) {}
    ValueType(Repr repr, std::shared_ptr<ValueType> child) : repr(repr), child(child){};

    std::string getRepr() const
    {
        if (child != nullptr) {
            return getReprStr() + "<" + child->getRepr() + ">";
        } else {
            return getReprStr();
        }
    }

    template <typename T>
    static constexpr bool is_bool()
    {
        return std::same_as<T, bool>;
    }
    template <typename T>
    static constexpr bool is_int()
    {
        return !ValueType::is_bool<T>() && std::integral<T>;
    }
    template <typename T>
    static constexpr bool is_float()
    {
        return std::floating_point<T>;
    }
    template <typename T>
    static constexpr bool is_string()
    {
        return std::convertible_to<T, std::string>;
    }
    template <typename T>
    requires std::same_as<T, std::vector<typename T::value_type>>
               || std::same_as<T, std::array<typename T::value_type, std::tuple_size<T>::value>>
    static constexpr bool is_vector()
    {
        return true;
    }
    template <typename T>
    static constexpr bool is_vector()
    {
        return false;
    }

    template <typename T>
    static ValueType of()
    {
        if constexpr (ValueType::is_bool<T>()) {
            return ValueType(Repr::bool_);
        } else if constexpr (ValueType::is_int<T>()) {
            return ValueType(Repr::int_);
        } else if constexpr (ValueType::is_float<T>()) {
            return ValueType(Repr::float_);
        } else if constexpr (ValueType::is_string<T>()) {
            return ValueType(Repr::string_);
        } else if constexpr (ValueType::is_vector<T>()) {
            return ValueType::vectorOf(ValueType::of<typename T::value_type>());
        } else {
            return ValueType(Repr::unknown_);
        }
    }
    static ValueType vectorOf(const ValueType& v)
    {
        return ValueType(Repr::vector_, std::make_shared<ValueType>(v));
    }
};


}  // namespace WebCFace

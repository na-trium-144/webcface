#pragma once
#include <string>
#include <vector>
#include <pybind11/stl.h>
#include <webcface/type.hpp>
#include <webcface/serialize.hpp>
#include <webcface/deserialize.hpp>
#include <iostream>
namespace WebCFace
{
namespace py = pybind11;
inline ValueType getValueTypePy(const std::string& typename_py)
{
    if (typename_py.starts_with("<class '") && typename_py.ends_with("'>")) {
        // <class 'int'> → int
        return getValueTypePy(typename_py.substr(8, typename_py.length() - 10));
    } else if (typename_py == "int") {
        return ValueType::of<int>();
    } else if (typename_py == "float") {
        return ValueType::of<float>();
    } else if (typename_py == "bool") {
        return ValueType::of<bool>();
    } else if (typename_py == "str") {
        return ValueType::of<std::string>();
    } else if (typename_py.starts_with("list[") && typename_py.ends_with("]")) {
        const std::string child_typename_py = typename_py.substr(5, typename_py.length() - 6);
        return ValueType::vectorOf(getValueTypePy(child_typename_py));
    } else {
        std::cerr << "[WebCFace] unknown type " << typename_py << std::endl;
        return ValueType();
    }
}
inline ValueType getValueTypePy(pybind11::object type_py)
{
    const std::string typename_py = type_py.cast<pybind11::str>();
    return getValueTypePy(typename_py);
}
inline pybind11::object convertToPy(const ValueType& type, const Json::Value& value)
{
    if (type.repr == ValueType::Repr::int_) {
        return pybind11::int_(deserialize<std::int64_t>(value));
#warning "↑uint64の場合は?"
    } else if (type.repr == ValueType::Repr::float_) {
        return pybind11::float_(deserialize<double>(value));
    } else if (type.repr == ValueType::Repr::bool_) {
        return pybind11::bool_(deserialize<bool>(value));
    } else if (type.repr == ValueType::Repr::string_) {
        return pybind11::str(deserialize<std::string>(value));
    } else if (type.repr == ValueType::Repr::vector_) {
        pybind11::list l;
        for (std::size_t i = 0; i < value.size(); i++) {
            l.append(convertToPy(*type.child, value[static_cast<int>(i)]));
        }
        return l;
    } else {
        return pybind11::str(deserialize<std::string>(value));  // てきとうにstrにでもしておく
    }
}

inline Json::Value convertFromPy(const ValueType& type, const pybind11::object& value)
{
    if (type.repr == ValueType::Repr::int_) {
        return serialize<std::int64_t>(value.cast<pybind11::int_>());
    } else if (type.repr == ValueType::Repr::float_) {
        return serialize<double>(value.cast<pybind11::float_>());
    } else if (type.repr == ValueType::Repr::bool_) {
        return serialize<bool>(value.cast<pybind11::bool_>());
    } else if (type.repr == ValueType::Repr::string_) {
        return serialize<std::string>(value.cast<pybind11::str>());
    } else if (type.repr == ValueType::Repr::vector_) {
        Json::Value l(Json::arrayValue);
        for (std::size_t i = 0; i < pybind11::len(value); i++) {
            l.append(convertFromPy(*type.child, value.cast<pybind11::list>()[i]));
        }
        return l;
    } else {
        return serialize<std::string>(value.cast<pybind11::str>());  // てきとう
    }
}
inline Json::Value convertFromPy(const pybind11::object& type, const pybind11::object& value)
{
    return convertFromPy(getValueTypePy(type), value);
}
inline Json::Value convertFromPy(const pybind11::object& value)
{
    return convertFromPy(value.get_type().cast<py::object>(), value);
}

}  // namespace WebCFace
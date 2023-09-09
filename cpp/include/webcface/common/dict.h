#pragma once
#include <string>
#include <unordered_map>
#include <initializer_list>
#include <memory>
#include <vector>
#include "vector.h"

namespace WebCFace {
inline namespace Common {

template <typename T>
struct Dict;

//! keyとvalue(TまたはDict<T>)の1ペア
template <typename T>
struct DictElement {
    std::string key;
    std::shared_ptr<Dict<T>> child;

    DictElement() = default;
    DictElement(const std::string &key, const T &value);
    DictElement(const std::string &key, std::initializer_list<DictElement> li);
};

// DictElementのリスト or 値そのもの
template <typename T>
struct Dict {
    std::optional<T> value;
    std::unordered_map<std::string, std::shared_ptr<Dict<T>>> children;
    Dict() = default;
    Dict(const T &value) : value(value) {}
    Dict(std::initializer_list<DictElement<T>> li) {
        for (const auto &el : li) {
            children.emplace(el.key, el.child);
        }
    }

    Dict &operator[](const std::string &key) {
        auto p = key.find('.');
        if (p != std::string::npos) {
            auto key1 = key.substr(0, p), key2 = key.substr(p + 1);
            if (!children.count(key1)) {
                children.emplace(key1, std::make_shared<Dict>());
            }
            return (*children[key1])[key2];
        } else {
            if (!children.count(key)) {
                children.emplace(key, std::make_shared<Dict>());
            }
            return *children[key];
        }
    }
    Dict &operator[](const std::string &key) const {
        auto p = key.find('.');
        if (p != std::string::npos) {
            return (*children.at(key.substr(0, p)))[key.substr(p + 1)];
        } else {
            return *children.at(key);
        }
    }

    T get() const { return value.value(); }
    operator T() const { return value.value(); }

    template <typename U>
    using IsVectorOpt = typename std::enable_if<
        std::is_same_v<U, VectorOpt<typename U::value_type>>>::type;

    template <typename U = T, typename = IsVectorOpt<U>>
    operator typename U::value_type() const {
        return value.value();
    }

    template <typename U = T, typename = IsVectorOpt<U>>
    std::vector<typename U::value_type> getVec() const {
        return value.value();
    }
    template <typename U = T, typename = IsVectorOpt<U>>
    operator std::vector<typename U::value_type>() const {
        return value.value();
    }
};

template <typename T>
DictElement<T>::DictElement(const std::string &key, const T &value)
    : key(key), child(std::make_shared<Dict<T>>(value)) {}

template <typename T>
DictElement<T>::DictElement(const std::string &key,
                            std::initializer_list<DictElement> li)
    : key(key), child(std::make_shared<Dict<T>>(li)) {}

} // namespace Common
} // namespace WebCFace
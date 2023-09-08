#pragma once
#include <string>
#include <unordered_map>
#include <initializer_list>
#include <memory>
#include <vector>

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
    Dict(std::initializer_list<DictElement<T>> li){
        for(const auto &el: li){
            children.emplace(el.key, el.child);
        }
    }

    Dict &operator[](const std::string &key) const {
        return children.at(key);
    }
    T get() const { return value.value(); }
    operator T() const { return value.value(); }
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
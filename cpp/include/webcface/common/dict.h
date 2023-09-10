#pragma once
#include <string>
#include <unordered_map>
#include <initializer_list>
#include <optional>
#include <memory>
#include <vector>
#include "vector.h"

namespace WebCFace {
inline namespace Common {

template <typename T>
struct Dict;

//! keyとvalueの1ペア
template <typename T>
struct DictElement {
    std::string key;
    std::optional<T> value;
    std::initializer_list<DictElement> children;

    DictElement() = default;
    DictElement(const std::string &key, const T &value)
        : key(key), value(value) {}
    DictElement(const std::string &key, std::initializer_list<DictElement> li)
        : key(key), children(li) {}
};

template <typename T>
struct DictTraits {
    template <typename T1, typename T2>
    static T get(const T1 &children, const T2 &search_base_key) {
        return children->at(search_base_key);
    }
    template <typename T1, typename T2>
    static void set(const T1 &children, const T2 &search_base_key,
                    const T &val) {
        children->operator[](search_base_key) = val;
    }
    using ValueType = T;
};
template <typename T>
struct DictTraits<std::shared_ptr<T>> {
    template <typename T1, typename T2>
    static T get(const T1 &children, const T2 &search_base_key) {
        return *DictTraits<T>::get(children, search_base_key);
    }
    template <typename T1, typename T2>
    static void set(const T1 &children, const T2 &search_base_key,
                    const T &val) {
        DictTraits<T>::set(children, search_base_key, std::make_shared<T>(val));
    }
    using ValueType = T;
};
template <typename T>
struct DictTraits<VectorOpt<T>> {
    template <typename T1, typename T2>
    static T get(const T1 &children, const T2 &search_base_key) {
        return DictTraits<T>::get(children, search_base_key).value_first;
    }
    template <typename T1, typename T2>
    static std::vector<T> getVec(const T1 &children,
                                 const T2 &search_base_key) {
        return DictTraits<T>::get(children, search_base_key).vec;
    }
    template <typename T1, typename T2>
    static void set(const T1 &children, const T2 &search_base_key,
                    const T &val) {
        DictTraits<T>::set(children, search_base_key, VectorOpt<T>{val});
    }
    template <typename T1, typename T2>
    static void setVec(const T1 &children, const T2 &search_base_key,
                       const std::vector<T> &val) {
        DictTraits<T>::set(children, search_base_key, VectorOpt<T>{val});
    }
    using ValueType = T;
    using VecType = std::vector<T>;
};

template <typename T>
class Dict {
    // Tがshared_ptrの場合とか、値に破壊的変更をしてはいけない
    std::shared_ptr<std::unordered_map<std::string, T>> children;
    // operator[]などのアクセスのときにつけるprefix (末尾ピリオドを含まない)
    std::string search_base_key;

    void emplace(std::initializer_list<DictElement<T>> li,
                 const std::string &em_base_key = "") {
        for (const auto &el : li) {
            std::string key = el.key;
            if (!em_base_key.empty()) {
                key = em_base_key + "." + el.key;
            }
            if (el.value) {
                children->emplace(key, *el.value);
            } else {
                emplace(el.children, key);
            }
        }
    }

  public:
    Dict() : children(std::make_shared<std::unordered_map<std::string, T>>()) {}
    Dict(const std::shared_ptr<std::unordered_map<std::string, T>> &children,
         const std::string &search_base_key)
        : children(children), search_base_key(search_base_key) {}
    //! initializer_listから値をセットする場合のコンストラクタ
    Dict(std::initializer_list<DictElement<T>> li) : Dict() { emplace(li); }

    Dict operator[](const std::string &key) const {
        return Dict{children, search_base_key + "." + key};
    }
    auto getChildren() const {
        std::unordered_map<std::string, Dict> ds;
        for (const auto &it : *children) {
            if (it.first.starts_with(search_base_key + ".")) {
                auto p2 = it.first.find('.', search_base_key.size() + 1);
                if (p2 == std::string::npos) {
                    p2 = it.first.size();
                }
                std::string next_key =
                    it.first.substr(search_base_key.size() + 1,
                                    p2 - (search_base_key.size() + 1));
                ds.emplace(next_key, Dict{children, it.first.substr(0, p2)});
            }
        }
        return ds;
    }

    bool hasValue() const { return children.count(search_base_key); }

    auto get() const { return DictTraits<T>::get(children, search_base_key); }
    operator typename DictTraits<T>::ValueType() const { return get(); }

    template <typename U = T>
    auto getVec() const {
        return DictTraits<T>::getVec(children, search_base_key);
    }
    template <typename U = T>
    operator typename DictTraits<U>::VecType() const {
        return getVec();
    }

    Dict &set(const typename DictTraits<T>::ValueType &val) {
        DictTraits<T>::set(children, search_base_key, val);
        return *this;
    }
    Dict &operator=(const typename DictTraits<T>::ValueType &val) {
        set(val);
        return *this;
    }
    template <typename U = T>
    Dict &setVec(const typename DictTraits<U>::VecType &val) {
        DictTraits<U>::setVec(children, search_base_key, val);
        return *this;
    }
    template <typename U = T>
    Dict &operator=(const typename DictTraits<U>::VecType &val) {
        setVec(val);
        return *this;
    }
};

} // namespace Common
} // namespace WebCFace
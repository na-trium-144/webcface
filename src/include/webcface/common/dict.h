#pragma once
#include <string>
#include <unordered_map>
#include <initializer_list>
#include <optional>
#include <memory>
#include <vector>
#include <type_traits>
#include "vector.h"

namespace WebCFace {
inline namespace Common {

template <typename T>
struct DictTraits {
    using ValueType = T;
    static ValueType parse(const T &val) { return val; }
    static T wrap(const ValueType &val) { return val; }
    using VecType = void;
};
template <typename T>
struct DictTraits<std::shared_ptr<T>> {
    using ValueType = typename DictTraits<T>::ValueType;
    static ValueType parse(const std::shared_ptr<T> &val) {
        return DictTraits<T>::parse(*val);
    }
    using VecType = typename DictTraits<T>::VecType;
    template <typename U = T>
    static VecType parseVec(const std::shared_ptr<U> &val) {
        return DictTraits<U>::parseVec(*val);
    }

    static std::shared_ptr<T> wrap(const ValueType &val) {
        return std::make_shared<T>(DictTraits<T>::wrap(val));
    }
    template <typename V = VecType, typename = typename std::enable_if<
                                        !std::is_same_v<V, void>>::type>
    static std::shared_ptr<T> wrapVec(const V &val) {
        return std::make_shared<T>(DictTraits<T>::wrapVec(val));
    }
};
template <typename T>
struct DictTraits<VectorOpt<T>> {
    using ValueType = typename DictTraits<T>::ValueType;
    static ValueType parse(const VectorOpt<T> &val) {
        return DictTraits<T>::parse(val);
    }
    using VecType = std::vector<T>;
    static VecType parseVec(const VectorOpt<T> &val) { return val; }

    static VectorOpt<T> wrap(const ValueType &val) {
        return DictTraits<T>::wrap(val);
    }
    static VectorOpt<T> wrapVec(const VecType &val) { return val; }
};

//! keyとvalueの1ペア
template <typename T>
struct DictElement {
    std::string key;
    std::optional<T> value;
    std::initializer_list<DictElement> children;

    DictElement() = default;
    DictElement(const std::string &key,
                const typename DictTraits<T>::ValueType &value)
        : key(key), value(DictTraits<T>::wrap(value)) {}
    template <typename U = T>
    DictElement(const std::string &key,
                const typename DictTraits<U>::VecType &value)
        : key(key), value(DictTraits<U>::wrapVec(value)) {}
    // template <typename U = T>
    // DictElement(const std::string &key,
    //             std::initializer_list<typename
    //             DictTraits<U>::VecType<U>::value_type> value)
    //     : key(key), value(DictTraits<U>::wrapVec(value)) {}
    DictElement(const std::string &key, std::initializer_list<DictElement> li)
        : key(key), children(li) {}
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
        std::string new_key = key;
        if (!search_base_key.empty()) {
            new_key = search_base_key + "." + key;
        }
        return Dict{children, new_key};
    }
    Dict operator[](const char *key) const { return (*this)[std::string(key)]; }
    
    auto getChildren() const {
        std::unordered_map<std::string, Dict> ds;
        std::string search_base_key_dot = "";
        if (!search_base_key.empty()) {
            search_base_key_dot = search_base_key + ".";
        }
        for (const auto &it : *children) {
            if (it.first.starts_with(search_base_key_dot)) {
                auto p2 = it.first.find('.', search_base_key_dot.size());
                if (p2 == std::string::npos) {
                    p2 = it.first.size();
                }
                std::string next_key =
                    it.first.substr(search_base_key_dot.size(),
                                    p2 - (search_base_key_dot.size()));
                ds.emplace(next_key, Dict{children, it.first.substr(0, p2)});
            }
        }
        return ds;
    }

    bool hasValue() const { return children->count(search_base_key); }
    T getRaw() const { return children->at(search_base_key); }

    auto get() const {
        return DictTraits<T>::parse(children->at(search_base_key));
    }
    operator typename DictTraits<T>::ValueType() const { return get(); }

    template <typename U = T>
    auto getVec() const {
        return DictTraits<U>::parseVec(children->at(search_base_key));
    }
    template <typename U = T>
    operator typename DictTraits<U>::VecType() const {
        return getVec();
    }

    Dict &set(const T &val) {
        (*children)[search_base_key] = val;
        return *this;
    }
    template <typename U = T,
              typename = typename std::enable_if<
                  !std::is_same_v<typename DictTraits<U>::ValueType, T>>::type>
    Dict &set(const typename DictTraits<U>::ValueType &val) {
        (*children)[search_base_key] = DictTraits<U>::wrap(val);
        return *this;
    }
    template <typename U = T>
    Dict &setVec(const typename DictTraits<U>::VecType &val) {
        (*children)[search_base_key] = DictTraits<U>::wrapVec(val);
        return *this;
    }
    template <typename U>
    Dict &operator=(const U &val) {
        set(val);
        return *this;
    }
};

} // namespace Common
} // namespace WebCFace
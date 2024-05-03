#pragma once
#include <string>
#include <unordered_map>
#include <initializer_list>
#include <optional>
#include <memory>
#include <vector>
#include <type_traits>
#include <concepts>
#include "vector.h"
#include <webcface/common/def.h>

WEBCFACE_NS_BEGIN
inline namespace Common {

struct VecTypeDisabled {};
//! 内部データ(T)とユーザーが取得したいデータ(ValueType)を相互変換するTrait
//! (T=ValueTypeの場合そのまま返す)
template <typename T>
struct DictTraits {
    using ValueType = T;
    static ValueType parse(const T &val) { return val; }
    static T wrap(const ValueType &val) { return val; }
    using VecType = VecTypeDisabled;
};
//! 内部データ(std::shared_ptr<T>)とユーザーが取得したいデータ(ValueType)を相互変換するTrait
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
    template <typename V = VecType>
    requires(!std::same_as<V, VecTypeDisabled>) static std::shared_ptr<
        T> wrapVec(const V &val) {
        return std::make_shared<T>(DictTraits<T>::wrapVec(val));
    }
};
//! 内部データ(VectorOpt<T>)とユーザーが取得したいデータ(ValueType)を相互変換するTrait
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
    //! {key, value} から変換するコンストラクタ
    template <typename U>
    requires std::convertible_to<U, typename DictTraits<T>::ValueType>
    DictElement(const std::string &key, const U &value)
        : key(key),
          value(DictTraits<T>::wrap(
              static_cast<typename DictTraits<T>::ValueType>(value))) {}
    //! {key, {value, value, ...}} から変換するコンストラクタ
    //! (DictTraits<T>が配列を受け付けるときのみ)
    template <typename U = T>
    DictElement(const std::string &key,
                const typename DictTraits<U>::VecType &value)
        : key(key), value(DictTraits<U>::wrapVec(value)) {}
    //! ネストしたinitializer_list
    DictElement(const std::string &key, std::initializer_list<DictElement> li)
        : key(key), children(li) {}
};

/*!
 * \brief 値の型をTに制限した、連想配列もどき
 *
 * T型の値1つ または 複数の子要素(名前とDictのペア)を持つ
 *
 */
template <typename T>
class Dict {
    /*!
     * Tがshared_ptrの場合があるので、値に破壊的変更をしてはいけない
     *
     */
    std::shared_ptr<std::unordered_map<std::string, T>> children;
    //! operator[]などのアクセスのときにつけるprefix (末尾ピリオドを含まない)
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
    //! クライアントの内部データからDictを生成する場合のコンストラクタ
    Dict(const std::shared_ptr<std::unordered_map<std::string, T>> &children,
         const std::string &search_base_key)
        : children(children), search_base_key(search_base_key) {}
    //! initializer_listから値をセットする場合のコンストラクタ
    Dict(std::initializer_list<DictElement<T>> li) : Dict() { emplace(li); }

    //! 要素にアクセスする
    Dict operator[](const std::string &key) const {
        std::string new_key = key;
        if (!search_base_key.empty()) {
            new_key = search_base_key + "." + key;
        }
        return Dict{children, new_key};
    }
    //! 要素にアクセスする
    Dict operator[](const char *key) const { return (*this)[std::string(key)]; }

    //! 要素のリストを返す
    std::unordered_map<std::string, Dict> getChildren() const {
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

    //! この要素が値を持っているかどうかを返す
    bool hasValue() const { return children->count(search_base_key); }
    //! 内部のデータ型のまま値を返す
    T getRaw() const { return children->at(search_base_key); }

    //! 値を返す
    typename DictTraits<T>::ValueType get() const {
        return DictTraits<T>::parse(children->at(search_base_key));
    }
    //! 値型にキャストすることで値を返す
    operator typename DictTraits<T>::ValueType() const { return get(); }

    //! 値を配列で返す
    template <typename U = T>
    typename DictTraits<U>::VecType getVec() const {
        return DictTraits<U>::parseVec(children->at(search_base_key));
    }
    //! 値を配列にキャストすることで返す
    template <typename U = T>
    operator typename DictTraits<U>::VecType() const {
        return getVec();
    }

    //! 値(内部データ型)を代入する
    Dict &set(const T &val) {
        (*children)[search_base_key] = val;
        return *this;
    }
    //! 値を代入する
    template <typename U = T,
              typename = typename std::enable_if<
                  !std::is_same_v<typename DictTraits<U>::ValueType, T>>::type>
    Dict &set(const typename DictTraits<U>::ValueType &val) {
        (*children)[search_base_key] = DictTraits<U>::wrap(val);
        return *this;
    }
    //! 値(配列)を代入する
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
WEBCFACE_NS_END

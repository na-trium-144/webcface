#pragma once
#include <cstddef>
#include <istream>
#include <string>
#include <tuple>
#include <vector>
#include <functional>
#include <iostream>
#include <sstream>
#include <cassert>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/json.h>

namespace WebCFace
{
//! 値をjsonに変換する
/*! \param val 変換する値\n
 * 整数、実数、bool、文字列: そのままJson::Valueに変換可能\n
 * std::vector, std::array: 内容を再帰的にserializeする\n
 */
template <typename T>
Json::Value serialize(const T& val)
{
    if constexpr (std::is_convertible_v<T, Json::Value>) {
        return val;
    } else if constexpr (std::is_same_v<T, std::vector<typename T::value_type>>) {
        using V = typename T::value_type;
        const std::size_t size = val.size();
        Json::Value arr(Json::arrayValue);
        for (std::size_t i = 0; i < size; i++) {
            arr[static_cast<int>(i)] = serialize(val[i]);
        }
        return arr;
    } else if constexpr (std::is_same_v<T,
                             std::array<typename T::value_type, std::tuple_size<T>::value>>) {
        using V = typename T::value_type;
        constexpr std::size_t a_size = std::tuple_size<T>::value;
        Json::Value arr;
        for (std::size_t i = 0; i < a_size; i++) {
            arr[static_cast<int>(i)] = serialize(val[i]);
        }
        return arr;
    } else {
        // static_assert(false && "cannot convert to Json");
    }
}

template <int n>
static void serializeFromVars(Json::Value& ar, const std::vector<std::string>& names)
{
    static_assert(n == 0);
}
//! serialize_multiの内部実装
/*!
 * \param n 変数の数
 * \param ar 変換後のJson
 * \param names それぞれの変数名
 * \param var, others... 変数
 */
template <int n, typename T, typename... U>
static void serializeFromVars(
    Json::Value& ar, const std::vector<std::string>& names, const T& var, const U&... others)
{
    if constexpr (n > 0) {
        ar[names[names.size() - n]] = serialize(var);
        serializeFromVars<n - 1>(ar, names, others...);
    }
}

//! 複数の変数をまとめてserializeする
/*!
 * \param names それぞれの変数名
 * \param var... 変換する変数
 */
template <typename... T>
Json::Value serialize_multi(const std::vector<std::string>& names, const T&... var)
{
    Json::Value res;
    constexpr std::size_t return_num = sizeof...(var);
    serializeFromVars<return_num>(res, names, var...);
    return res;
}

//! 変数(参照渡し)をjsonに変換するコールバックを生成
/*! \param names 各変数の変数名
 * \param var... 変数
 * \return callback 実行するとその変数をjson化したものを返す
 */
template <typename... T>
std::function<Json::Value()> getCallbackVarToJson(const std::vector<std::string>& names, T&... var)
{
    if constexpr (sizeof...(var) == 1) {
        return [&var...]() { return serialize(var...); };
    } else {
        assert(sizeof...(var) == names.size());
        return [&var..., names]() { return serialize_multi(names, var...); };
    }
}

//! 変数(値渡し)をjsonに変換するコールバックを生成
/*! \param names 各変数の変数名
 * \param var... 変数
 * \return callback 実行するとその変数をjson化したものを返す
 */
template <typename... T>
std::function<Json::Value()> getCallbackValueToJson(const std::vector<std::string>& names, T&... var)
{
    if constexpr (sizeof...(var) == 1) {
        return [var...]() { return serialize(var...); };
    } else {
        assert(sizeof...(var) == names.size());
        return [var..., names]() { return serialize_multi(names, var...); };
    }
}

//! 関数の戻り値をjsonに変換するコールバックを生成
/*! \param callback
 * \return callback 実行するとcallbackを実行し、その戻り値をjson化したものを返す
 */
template <typename T>
std::function<Json::Value()> getCallbackFunctionReturnToJson(std::function<T()> callback)
{
    return [callback]() { return serialize(callback()); };
}

}  // namespace WebCFace

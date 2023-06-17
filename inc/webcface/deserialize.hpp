#pragma once
#include <cstddef>
#include <istream>
#include <type_traits>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
#include <functional>
#include <iostream>
#include <jsoncpp/json/json.h>

namespace WebCFace
{
//! jsonから型Tに変換する
/*! \param val 変換するJsonデータ\n
 * 整数、実数、bool、文字列: そのままTに変換可能\n
 * std::vector, std::array: 内容を再帰的にdeserializeする\n
 */
template <typename T>
T deserialize(const Json::Value& val)
{
    if constexpr (std::is_convertible_v<T, Json::Value>) {
        return val.as<T>();
    } else if constexpr (std::is_same_v<T, std::vector<typename T::value_type>>) {
        using V = typename T::value_type;
        const std::size_t size = val.size();
        std::vector<V> ret;
        for (std::size_t i = 0; i < size; i++) {
            ret.push_back(deserialize<V>(val[static_cast<int>(i)]));
        }
        return ret;
    } else if constexpr (std::is_same_v<T,
                             std::array<typename T::value_type, std::tuple_size<T>::value>>) {
        using V = typename T::value_type;
        constexpr std::size_t a_size = std::tuple_size<T>::value;
        const std::size_t j_size = val.size() < a_size ? val.size() : a_size;
        std::array<V, a_size> ret;
        for (std::size_t i = 0; i < j_size; i++) {
            ret[i] = deserialize<V>(val[static_cast<int>(i)]);
        }
        return ret;
    }else{
        // static_assert(false && "cannot convert from Json");
    }
}

//! tupleとして複数の変数にまとめてdeserializeする
/*!
 * \param n 変数の数
 * \param ar 変換するJson
 * \param names それぞれの変数名
 * \param tuple 値を格納するtuple
 */
template <int n, typename T>
void deserializeToTuple(const Json::Value& ar, const std::vector<std::string>& names, T& tuple)
{
    if constexpr (n > 0) {
        const auto val = ar[names[n - 1]];
        using type = typename std::tuple_element<n - 1, T>::type;
        std::get<n - 1>(tuple) = deserialize<type>(val);
        deserializeToTuple<n - 1>(ar, names, tuple);
    }
}
template <int n>
void deserializeToVars(
    const Json::Value& ar, const std::vector<std::string>& names)
{
    static_assert(n == 0);
}
//! 複数の変数にまとめてdeserializeする
/*!
 * \param n 変数の数
 * \param ar 変換するJson
 * \param names それぞれの変数名
 * \param var, others... 値を格納する変数
 */
template <int n, typename T, typename... Others>
void deserializeToVars(
    const Json::Value& ar, const std::vector<std::string>& names, T& var, Others&... others)
{
    if constexpr (n > 0) {
        const auto val = ar[names[names.size() - n]];
        var = deserialize<T>(val);
        deserializeToVars<n - 1>(ar, names, others...);
    }
}

//! Jsonをdeserializeしcallbackの引数に渡して実行するコールバックを生成
/*! \param names 各引数の変数名
 * \param callback 引数型は deserialize() 可能であれば任意
 * \return callback 引数にjsonを文字列で渡して実行すると、callbackの引数
 */
template <typename... Args>
std::function<void(const std::string&)> getCallbackJsonToFunction(
    const std::vector<std::string>& names, std::function<void(Args...)> callback)
{
    constexpr std::size_t args_num = std::tuple_size<std::tuple<Args...>>::value;
    // constexpr std::size_t args_num = sizeof...(Args);
    return [names, callback](const std::string& args_json) {
        std::tuple<Args...> args_tuple;  // jsonのパース結果を受け取るtuple
        if (args_num) {
            Json::Value parsed(Json::objectValue);
            Json::Reader reader;
            reader.parse(args_json, parsed);

            deserializeToTuple<args_num>(parsed, names, args_tuple);
        }
        std::apply(callback, args_tuple);  // callbackの引数にtupleを渡す
    };
}
inline auto getCallbackJsonToFunction_withJsonMap(const std::vector<std::string>& names,
    std::function<void(std::map<std::string, Json::Value>)> callback)
{
    return [names, callback](const std::string& args_json) {
        std::map<std::string, Json::Value> args_map;
        Json::Value parsed(Json::objectValue);
        Json::Reader reader;
        reader.parse(args_json, parsed);
        for (const auto& n : names) {
            args_map[n] = parsed[n];
        }
        callback(args_map);
    };
}

template <typename... T>
auto getCallbackJsonToVar(const std::vector<std::string>& names, T&... var)
{
    // constexpr std::size_t args_num = std::tuple_size<std::tuple<Args...>>::value;
    constexpr std::size_t args_num = sizeof...(var);
    return [names, &var...](const std::string& args_json) {
        Json::Value parsed(Json::objectValue);
        Json::Reader reader;
        reader.parse(args_json, parsed);
        deserializeToVars<args_num>(parsed, names, var...);
    };
}
}  // namespace WebCFace

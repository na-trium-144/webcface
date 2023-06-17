#pragma once

#include <tuple>
#include <webcface/serialize.hpp>
#include <webcface/deserialize.hpp>
#include <webcface/server.hpp>
#include <webcface/type.hpp>

namespace WebCFace
{
inline namespace Registration
{
//! 名前の指定が不足している場合に"unknown0"等で埋める
inline void fillNameVector(std::vector<std::string>& names, const std::size_t num)
{
    for (std::size_t i = 0; names.size() < num; i++) {
        names.push_back("unknown" + std::to_string(i));
    }
    if (names.size() > num) {
        names.resize(num);
    }
}

//! 無名の関数に"noname.0"などの名前をつける
inline std::string getNewFunctionName()
{
    static int count = 0;
    return "noname." + std::to_string(count++);
}


template <typename... Args>
void addFunctionToRobot_impl(
    std::string name, std::function<void(Args...)> callback, std::vector<std::string> arg_names)
{
    // auto arg_names2 = arg_names; どうせコピーするなら最初から引数をコピーで取ればよい
    constexpr std::size_t args_num = sizeof...(Args);
    fillNameVector(arg_names, args_num);

    auto pr_callback = getCallbackJsonToFunction(arg_names, callback);
    std::vector<ValueType> types = {ValueType::of<Args>()...};
    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (to_robot_func.find(name) == to_robot_func.end());
        to_robot_func[name] = ToRobotInfo{pr_callback, std::move(arg_names), types};
        if (is_new) {
            setting_changed = true;
        }
    }
}

//! pybindからの関数登録の実装で使用

inline void addFunctionToRobot_withJsonMap(std::string name,
    std::function<void(std::map<std::string, Json::Value>)> callback,
    std::vector<std::string> arg_names, std::vector<ValueType> arg_types)
{
    auto pr_callback = getCallbackJsonToFunction_withJsonMap(arg_names, callback);
    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (to_robot_func.find(name) == to_robot_func.end());
        to_robot_func[name] = ToRobotInfo{pr_callback, std::move(arg_names), std::move(arg_types)};
        if (is_new) {
            setting_changed = true;
        }
    }
}

//! 関数の登録
/*! フロントエンドから呼び出す関数を登録する。
 * \param name 関数名
 * 半角コロンは避ける
 * \param callback 関数(std::function, ラムダ式など)\n
 * 戻り値はvoidでなければならない\n
 * 引数は deserialize() ができる型のみ
 * \param arg_names 引数の名前 省略時や要素数不足時は"unknown0"のような名前が自動でつく
 */
template <typename T>
void addFunctionToRobot(
    const std::string& name, T callback, const std::vector<std::string>& arg_names = {})
{
    addFunctionToRobot_impl(name, std::function(callback), arg_names);
}

//! フロントエンドから値を変更する変数の登録(複数)
/*! 例えば name="test", names={"a", "b"} のとき"test.a" "test.b" の2つの変数が登録される。
 * \param name 全体変数名
 * \param names 各変数の変数名
 * \param var 変数
 * 参照をキャプチャするためスコープが切れる可能性のあるものは不可
 */
template <typename... T>
void addSharedVarToRobot(const std::string& name, std::vector<std::string> names, T&... var)
{
    constexpr std::size_t args_num = sizeof...(T);
    fillNameVector(names, args_num);

    auto pr_callback = getCallbackJsonToVar(names, var...);
    std::vector<ValueType> types = {ValueType::of<T>()...};
    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (to_robot_var.find(name) == to_robot_var.end());
        to_robot_var[name] = ToRobotInfo{pr_callback, names, types};
        if (is_new) {
            setting_changed = true;
        }
    }
}
//! フロントエンドから値を変更する変数の登録(単数)
/*!
 * \param name 変数名
 * \param var 変数
 * 参照をキャプチャするためスコープが切れる可能性のあるものは不可
 */
template <typename T>
void addSharedVarToRobot(const std::string& name, T& var)
{
    addSharedVarToRobot(name, {name}, var);
}

template <typename T>
void addFunctionFromRobot_impl(const std::string& name, std::function<T()> callback)
{
    auto pr_callback = getCallbackFunctionReturnToJson(callback);
    const auto type = ValueType::of<T>();
    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (from_robot.find(name) == from_robot.end());
        from_robot[name] = FromRobotInfo{pr_callback, {name}, {type}};
        if (is_new) {
            setting_changed = true;
        }
    }
}
//! pybindからの登録で使用

inline void addFunctionFromRobot_withJson(
    const std::string& name, std::function<Json::Value()> callback, ValueType return_type)
{
    std::lock_guard lock(internal_mutex);

    bool is_new = (from_robot.find(name) == from_robot.end());
    from_robot[name] = FromRobotInfo{callback, {name}, {return_type}};
    if (is_new) {
        setting_changed = true;
    }
}
//! フロントエンドに戻り値を送る関数の登録
/*!
 * \param name 関数名
 * \param callback 関数\n
 * 引数をもつことはできない\n
 * 戻り値は serialize() が取れる型でなければならない\n
 */
template <typename T>
void addFunctionFromRobot(const std::string& name, T callback)
{
    addFunctionFromRobot_impl(name, std::function(callback));
}

template <typename... T>
void addSharedVarFromRobot(const std::string& name, std::vector<std::string> var_names, T&... var)
{
    constexpr std::size_t var_num = sizeof...(var);
    fillNameVector(var_names, var_num);
    auto pr_callback = getCallbackVarToJson(var_names, var...);
    std::vector<ValueType> types = {ValueType::of<T>()...};
    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (from_robot.find(name) == from_robot.end());
        from_robot[name] = FromRobotInfo{pr_callback, var_names, types};
        if (is_new) {
            setting_changed = true;
        }
    }
}
template <typename T>
void addSharedVarFromRobot(const std::string& name, const T& var)
{
    auto pr_callback = getCallbackVarToJson({name}, var);
    std::vector<ValueType> types = {ValueType::of<T>()};
    {

        std::lock_guard lock(internal_mutex);
        bool is_new = (from_robot.find(name) == from_robot.end());
        from_robot[name] = FromRobotInfo{pr_callback, {name}, types};
        if (is_new) {
            setting_changed = true;
        }
    }
}
template <typename... T>
void addValueFromRobot(const std::string& name, std::vector<std::string> var_names, const T&... var)
{
    constexpr std::size_t var_num = sizeof...(var);
    fillNameVector(var_names, var_num);
    auto pr_callback = getCallbackValueToJson(var_names, var...);
    std::vector<ValueType> types = {ValueType::of<T>()...};

    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (from_robot.find(name) == from_robot.end());
        from_robot[name] = FromRobotInfo{pr_callback, var_names, types};
        if (is_new) {
            setting_changed = true;
        }
    }
}
template <typename T>
void addValueFromRobot(const std::string& name, const T& var)
{
    auto pr_callback = getCallbackValueToJson({name}, var);
    std::vector<ValueType> types = {ValueType::of<T>()};


    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (from_robot.find(name) == from_robot.end());
        from_robot[name] = FromRobotInfo{pr_callback, {name}, types};
        if (is_new) {
            setting_changed = true;
        }
    }
}
inline void addValueFromRobot_withJson(
    const std::string& name, const Json::Value& var, ValueType value_type)
{
    auto pr_callback = std::function([=]() { return var; });
    {
        std::lock_guard lock(internal_mutex);

        bool is_new = (from_robot.find(name) == from_robot.end());

        from_robot[name] = FromRobotInfo{pr_callback, {name}, {value_type}};
        if (is_new) {
            setting_changed = true;
        }
    }
}

// srcはimgのsrcに直接渡される
// 画像のurl またはbase64エンコードした画像
inline void addImage(const std::string& name, const std::string& src)
{
    std::lock_guard lock(internal_mutex);

    bool is_new = (images.find(name) == images.end());
    images[name] = ImageInfo{serialize(src), true};
    if (is_new) {
        setting_changed = true;
    }
}
}  // namespace Registration
}  // namespace WebCFace

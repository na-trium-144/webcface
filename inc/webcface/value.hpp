#pragma once
#include <webcface/registration_impl.hpp>
#include <jsoncpp/json/json.h>
#include <string>
#include <vector>
#include <cstdint>
#include <concepts>

namespace WebCFace
{
inline namespace Registration
{
//! バックエンド→フロントエンドに送信する値
struct RegisterValue {
    std::string value_name;

    RegisterValue() : value_name(getNewFunctionName()) {}
    //! 値の名前を指定
    /*! \sa WebCFace::Literals::operator""_value()
     * \param value_name 名前
     */
    explicit RegisterValue(const std::string& value_name) : value_name(value_name) {}
    //! 値の名前を指定
    /*! \sa WebCFace::Literals::operator""_value()
     * \param value_name 名前
     */
    explicit RegisterValue(const char* value_name) : RegisterValue(std::string(value_name)) {}

    //! 関数を登録する(メソッドチェーン)
    /*! 登録した関数を WebCFace::sendData() 時に実行し、戻り値を送信する
     * \sa operator=()
     * \param callback 実行する関数\n
     * 引数なし\n
     * 戻り値は WebCFace::serialize() できる型
     * \return *this
     */
    template <std::invocable<> T>
    RegisterValue& value(const T& callback)
    {
        addFunctionFromRobot(value_name, callback);
        return *this;
    }
    //! 値を登録する(メソッドチェーン)
    /*! 値渡しで値を登録する
     * 値が変わるたびに繰り返し登録する必要があります
     * \sa operator=()
     * \param value 値 ( WebCFace::serialize() できる型)
     * \return *this
     */
    template <std::convertible_to<Json::Value> T>
    RegisterValue& value(const T& value)
    {
        addValueFromRobot<T>(value_name, value);
        return *this;
    }
    //! 変数の参照を登録する(メソッドチェーン)
    /*! 1度登録すれば値が変わったとき送信されます
     * \sa operator=()
     * \param value 変数のポインタ\n
     * WebCFace::serialize() できる型\n
     * スコープが切れる可能性のあるローカル変数は不可
     * \return *this
     */
    template <typename T>
        requires std::convertible_to<T, Json::Value>
                 && (!std::same_as<T, char>) && (!std::same_as<T, const char>)
    RegisterValue& value(T* value)
    {
        addSharedVarFromRobot(value_name, *value);
        return *this;
    }
    // todo
    // sharedポインタも良さそう


    //! 関数を登録する(コンストラクタ)
    /*! \sa value()
     */
    template <std::invocable<> T>
    RegisterValue(const T& callback) : RegisterValue()
    {
        this->value(callback);
    }
    //! 変数の参照を登録する(コンストラクタ)
    /*! \sa value()
     */
    template <typename T>
        requires std::convertible_to<T, Json::Value>
                 && (!std::same_as<T, char>) && (!std::same_as<T, const char>)
    RegisterValue(T* value) : RegisterValue()
    {
        this->value(value);
    }

    //! 関数、値、変数の参照を登録する(代入演算子)
    /*! \sa value()
     *  \return *this
     */
    template <typename T>
    RegisterValue& operator=(const T& value)
    {
        this->value(value);
        return *this;
    }

    //! レイアウト設定に渡すjsonを生成
    Json::Value json() const
    {
        Json::Value ret(Json::objectValue);
        ret["value_name"] = value_name;
        return ret;
    }
};
//! レイアウトで表示する「値」を表現する型
struct Displayable {
    Json::Value json;
    Displayable() : json(Json::objectValue) { json["value"] = Json::Value(Json::nullValue); }
    //! RegisterValue からキャストされる
    template <std::convertible_to<RegisterValue> T>
    Displayable(const T& value) : json(RegisterValue(value).json())
    {
    }
    //! そのまま表示する値
    /*!
     * \param value 値 整数、実数、bool、文字列など
     */
    template <typename T>
        requires std::convertible_to<T, Json::Value> && (!std::convertible_to<T, RegisterValue>)
    Displayable(const T& value) : Displayable()
    {
        json["value"] = serialize(value);
    }
    template <typename T>
    T as() const
    {
        return deserialize<T>(json["value"]);
    }
};

//! フロントエンドから呼び出す関数を表す型
struct RegisterCallback {
    std::string callback_name;
    std::vector<Displayable> args;  // 引数名または引数の値
    std::vector<Json::Value> default_values;

    RegisterCallback() : callback_name(getNewFunctionName()) {}
    //! 関数名を指定
    /*! \sa WebCFace::Literals::operator""_callback()
     * \param callback_name 関数名
     */
    explicit RegisterCallback(const std::string& callback_name) : callback_name(callback_name) {}
    //! 関数名を指定
    /*! \sa WebCFace::Literals::operator""_callback()
     * \param callback_name 関数名
     */
    explicit RegisterCallback(const char* callback_name)
        : RegisterCallback(std::string(callback_name))
    {
    }

    //! 関数を登録する(メソッドチェーン)
    //! \sa operator=()
    //! \param callback 関数 戻り値はもたない
    template <typename T>
    RegisterCallback& callback(T callback)
    {
        std::vector<std::string> arg_names;
        for (const auto& a : args) {
            arg_names.push_back(a.as<std::string>());
        }
        args.clear();
        addFunctionToRobot(callback_name, callback, arg_names, default_values);
        return *this;
    }

    //! 関数を登録する(コンストラクタ)
    //! \sa callback()
    template <typename T>
    RegisterCallback(const T& callback) : RegisterCallback()
    {
        this->callback(callback);
    }
    //! 関数を登録する(代入)
    //! \sa callback()
    template <typename T>
    RegisterCallback& operator=(const T& value)
    {
        this->callback(value);
        return *this;
    }

    RegisterCallback& arg() { return *this; }
    //! 引数名を設定 or 引数の値を設定 (メソッドチェーン)
    /*! callback() 設定より前に arg() が呼び出された場合、引数名を設定する意味になります\n
     * callback() より後に arg() が呼び出された場合、引数の値を設定する意味になります\n
     * 後者はレイアウト設定で使います
     * \param arg ↑の説明を参照
     * \sa operator<<()
     */
    template <typename... T>
    RegisterCallback& arg(const Displayable& arg, const T&... others)
    {
        this->args.push_back(arg);
        return this->arg(others...);
    }

    RegisterCallback& default_value() { return *this; }
    template <typename Arg, typename... T>
    RegisterCallback& default_value(const Arg& arg, const T&... others)
    {
        this->default_values.push_back(serialize(arg));
        return this->default_value(others...);
    }
    //! 引数名を設定 or 引数の値を設定 (<<演算子)
    //! \sa arg()
    RegisterCallback& operator<<(const Displayable& arg)
    {
        this->arg(arg);
        return *this;
    }

    //! レイアウト設定に渡す
    Json::Value json() const
    {
        Json::Value ret(Json::objectValue);
        ret["callback_name"] = callback_name;
        ret["args"] = Json::Value(Json::arrayValue);
        for (const auto& a : args) {
            ret["args"].append(a.json);
        }
        return ret;
    }
};
}  // namespace Registration

inline namespace Literals
{
//! WebCFace::Registration::RegisterCallback(callback_name) の省略形です
inline RegisterCallback operator""_callback(const char* callback_name, std::size_t)
{
    return RegisterCallback(callback_name);
}
//! WebCFace::Registration::RegisterValue(value_name) の省略形です
inline RegisterValue operator""_value(const char* value_name, std::size_t)
{
    return RegisterValue(value_name);
}

}  // namespace Literals

}  // namespace WebCFace
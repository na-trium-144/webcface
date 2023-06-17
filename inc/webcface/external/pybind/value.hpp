#pragma once
#include <webcface/external/pybind/registration_impl.hpp>
#include <webcface/external/pybind/cast.hpp>
#include <webcface/value.hpp>
#include <string>
namespace WebCFace::pybind
{
namespace py = pybind11;
inline namespace Registration
{
struct PyRegisterValue : RegisterValue {
    py::object return_type;
    //! コンストラクタ
    /*!
     * \param name 値の名前
     * \sa ret(), value()
     */
    explicit PyRegisterValue(const std::string& name = "", const py::object& ret = py::none(),
        const py::object& value = py::none())
    {
        if (name == "") {
            value_name = getNewFunctionName();
        } else {
            value_name = name;
        }
        if (!py::isinstance<py::none>(ret)) {
            this->ret(ret);
        }
        if (!py::isinstance<py::none>(value)) {
            this->value(value);
        }
    }
    //! コンストラクタ(キャスト用)
    /*!
     * \param value がPyRegisterValueのとき、コピーコンストラクタ\n
     * その他の型の場合、 value() に渡す(名前は設定できない)
     */
    explicit PyRegisterValue(const py::object& value)
    {
        if (py::isinstance<PyRegisterValue>(value)) {
            value_name = value.cast<PyRegisterValue>().value_name;
            return_type = value.cast<PyRegisterValue>().return_type;
        } else {
            value_name = getNewFunctionName();
            this->value(value);
        }
    }

    //! 戻り値の型を設定
    /*! 登録する値が関数の場合、戻り値の値を事前に設定する必要があります\n
     * 変数を登録する場合は自動で型を判別できるので、ret()は指定しなくても良いです
     * \param return_type 戻り値の型\n
     * int, str などのclassを渡すか、"int", "str"などの文字列を指定\n
     * listの場合は"list[int]"のようにlistの中身の型も指定
     */
    PyRegisterValue& ret(const py::object& return_type)
    {
        this->return_type = return_type;
        return *this;
    }
    //! 値や関数を登録
    //! \sa WebCFace::Registration::RegisterValue::value()
    PyRegisterValue& value(const py::object& value)
    {
        if (py::hasattr(value, "__call__")) {
            addFunctionFromRobotPy(value_name, value, return_type);
        } else {
            if (return_type.is_none()) {
                ret(value.get_type().cast<py::object>());
            }
            addValueFromRobotPy(value_name, value, return_type);
        }
        return *this;
    }
};
struct PyDisplayable : Displayable {
    //! PyRegisterValueからキャストされる
    explicit PyDisplayable(const py::object& value) : Displayable()
    {
        if (py::isinstance<PyDisplayable>(value)) {
            json = value.cast<PyDisplayable>().json;
        } else if (py::isinstance<PyRegisterValue>(value)) {
            json = value.cast<PyRegisterValue>().json();
        } else if (py::hasattr(value, "__call__")) {
            json = PyRegisterValue(value).json();
        } else {
            json["value"] = convertFromPy(value);
        }
    }
};

struct PyRegisterCallback : RegisterCallback {
    //! コンストラクタ
    /*!
     * \param name 関数の名前
     * \sa arg(), callback()
     */
    explicit PyRegisterCallback(const std::string& name = "", const py::object& arg = py::none(),
        const py::object& default_value = py::none(),
        const py::object callback = py::none())
    {
        if (name == "") {
            callback_name = getNewFunctionName();
        } else {
            callback_name = name;
        }
        if (py::isinstance<py::list>(arg)) {
            this->arg(arg.cast<py::list>());
        } else if (py::isinstance<py::dict>(arg)) {
            this->arg(arg.cast<py::dict>());
        } else if (!py::isinstance<py::none>(arg)) {
            std::cerr << "RegisterCallback: invalid arg type" << std::endl;
        }
        if (py::isinstance<py::list>(default_value)) {
            this->default_value(default_value.cast<py::list>());
        } else if (py::isinstance<py::dict>(default_value)) {
            this->default_value(default_value.cast<py::dict>());
        } else if (!py::isinstance<py::none>(default_value)) {
            std::cerr << "RegisterCallback: invalid default_value type" << std::endl;
        }
        if (!py::isinstance<py::none>(callback)) {
            this->callback(callback);
        }
    }
    //! コンストラクタ(キャスト用)
    /*!
     * \param value がPyRegisterCallbackのとき、コピーコンストラクタ\n
     * その他の型の場合、 callback() に渡す(名前は設定できない)
     */
    explicit PyRegisterCallback(const py::object& obj)
    {
        if (py::isinstance<PyRegisterCallback>(obj)) {
            callback_name = obj.cast<PyRegisterCallback>().callback_name;
            this->args = obj.cast<PyRegisterCallback>().args;
            this->arg_types = obj.cast<PyRegisterCallback>().arg_types;
        } else {
            callback_name = getNewFunctionName();
            this->callback(obj);
        }
    }

    std::vector<std::string> arg_names;
    std::vector<py::object> arg_types;
    std::vector<py::object> default_values;
    //! 引数名を設定 or 引数の値を設定 (メソッドチェーン)
    /*! callback() 設定より前に arg() が呼び出された場合、引数名を設定する意味になります\n
     * callback() より後に arg() が呼び出された場合、引数の値を設定する意味になります\n
     * 後者はレイアウト設定で使います
     * \param arg ↑の説明を参照\n
     * 前者の場合、dictで`{"引数名":型, "引数名":型, ...}`を指定
     * (型の表現方法は PyRegisterValue::ret() を参照)\n
     * 後者の場合、dictで`{"引数名":値, "引数名":値, ...}`またはlistで`[値, 値, ...]`を指定
     * \sa WebCFace::Registration::RegisterCallback::arg()
     */
    PyRegisterCallback& arg(const py::list& args)
    {
        if (this->args.size() < this->arg_types.size()) {
            // 引数の値を設定
            for (std::size_t i = 0; i < py::len(args); i++) {
                this->args.push_back(convertFromPy(arg_types[i], args[i]));
            }
        } else {
            for (std::size_t i = 0; i < py::len(args); i++) {
                this->args.push_back(args[i].cast<std::string>());
                this->arg_types.push_back(py::none());
            }
        }
        return *this;
    }
    //! 引数名を設定 or 引数の値を設定 (メソッドチェーン)
    /*! callback() 設定より前に arg() が呼び出された場合、引数名を設定する意味になります\n
     * callback() より後に arg() が呼び出された場合、引数の値を設定する意味になります\n
     * 後者はレイアウト設定で使います
     * \param arg ↑の説明を参照\n
     * 前者の場合、dictで`{"引数名":型, "引数名":型, ...}`を指定
     * (型の表現方法は PyRegisterValue::ret() を参照)\n
     * 後者の場合、dictで`{"引数名":値, "引数名":値, ...}`またはlistで`[値, 値, ...]`を指定
     * \sa WebCFace::Registration::RegisterCallback::arg()
     */
    PyRegisterCallback& arg(const py::dict& args)
    {
        if (this->args.size() < this->arg_types.size()) {
            // 引数の値を設定
            for (std::size_t i = 0; i < this->arg_names.size(); i++) {
                this->args.push_back(convertFromPy(arg_types[i], args[py::str(arg_names[i])]));
            }
        } else {
            auto names = args.attr("keys")().cast<py::list>();
            auto types = args.attr("values")().cast<py::list>();
            for (std::size_t i = 0; i < py::len(names); i++) {
                this->args.push_back(names[i].cast<std::string>());
                this->arg_types.push_back(types[i].cast<py::object>());
            }
        }
        return *this;
    }

    PyRegisterCallback& default_value(const py::list& args)
    {
        for (std::size_t i = 0; i < py::len(args); i++) {
            this->default_values.push_back(args[i]);
        }
        return *this;
    }
    PyRegisterCallback& default_value(const py::dict& args)
    {
        for (std::size_t i = 0; i < this->arg_names.size(); i++) {
            this->default_values.push_back(args[py::str(arg_names[i])]);
        }
        return *this;
    }

    //! 関数を登録する(メソッドチェーン)
    //! \sa WebCFace::Registration::RegisterCallback::callback()
    //! \param callback 関数 戻り値はもたない
    PyRegisterCallback& callback(const py::object& callback)
    {
        arg_names.clear();
        for (const auto& a : args) {
            arg_names.push_back(a.as<std::string>());
        }
        args.clear();
        addFunctionToRobotPy(callback_name, callback, arg_names, arg_types, default_values);
        return *this;
    }
};
}  // namespace Registration
}  // namespace WebCFace::pybind

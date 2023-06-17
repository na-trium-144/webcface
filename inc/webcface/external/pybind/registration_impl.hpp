#pragma once
#include <webcface/registration.hpp>
#include <webcface/type.hpp>
#include <webcface/external/pybind/cast.hpp>
#include <string>
#include <vector>
#include <pybind11/stl.h>
namespace WebCFace::pybind
{
inline namespace Registration
{
namespace py = pybind11;

inline py::module_ this_module;

void keep_object(const std::string& name, const py::object& obj)
{
    auto object_keeper = this_module.attr("object_keeper");
    if (!object_keeper.contains(name)) {
        object_keeper[name.c_str()] = py::list();
    }
    object_keeper[name.c_str()].attr("append")(obj);
    while (py::len(object_keeper[name.c_str()]) >= 5) {
        object_keeper[name.c_str()].attr("pop")(0);
    }
}

inline void addFunctionToRobotPy(std::string name, py::object callback,
    std::vector<std::string> arg_names, std::vector<py::object> arg_types_py)
{
    keep_object(name, callback);

    std::vector<ValueType> arg_types;
    for (int i = 0; i < arg_names.size(); i++) {
        arg_types.push_back(getValueTypePy(arg_types_py[i]));
    }
    addFunctionToRobot_withJsonMap(
        name,
        [=](std::map<std::string, Json::Value> args_map) {
            py::gil_scoped_acquire acquire;
            py::list args;
            for (int i = 0; i < arg_types.size(); i++) {
                args.append(convertToPy(arg_types[i], args_map[arg_names[i]]));
            }
            callback(*args);
        },
        arg_names, arg_types);
}
inline void addFunctionFromRobotPy(std::string name, py::object callback, py::object return_type_py)
{
    keep_object(name, callback);

    auto return_type = getValueTypePy(return_type_py);
    auto callback_cast = [=]() {
        py::gil_scoped_acquire acquire;
        return convertFromPy(return_type, callback());
    };
    addFunctionFromRobot_withJson(name, callback_cast, return_type);
}
inline void addValueFromRobotPy(std::string name, py::object value, py::object value_type_py)
{
    auto value_type = getValueTypePy(value_type_py);
    addValueFromRobot_withJson(name, convertFromPy(value_type, value), value_type);
}
}  // namespace Registration
}  // namespace WebCFace::pybind

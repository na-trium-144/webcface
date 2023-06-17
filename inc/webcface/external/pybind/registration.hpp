#pragma once
#include <webcface/external/pybind/registration_impl.hpp>
#include <webcface/external/pybind/value.hpp>
namespace WebCFace::pybind
{
namespace py = pybind11;
inline void defRegistration(py::module& m)
{
    using namespace py::literals;
    this_module = m;
    m.attr("object_keeper") = py::dict();
    py::class_<PyRegisterValue>(m, "RegisterValue")
        .def(py::init<const std::string&, const py::object&, const py::object&>(), "name"_a = "",
            "ret"_a = py::none(), "value"_a = py::none())
        .def(py::init<const py::object&>())
        .def("ret", &PyRegisterValue::ret)
        .def("value", py::overload_cast<const py::object&>(&PyRegisterValue::value));
    py::class_<PyRegisterCallback>(m, "RegisterCallback")
        .def(py::init<const std::string&, const py::object&, const py::object&, const py::object&>(), "name"_a = "",
            "arg"_a = py::none(), "default_value"_a = py::none(), "callback"_a = py::none())
        .def(py::init<const py::object&>())
        .def("arg", py::overload_cast<const py::list&>(&PyRegisterCallback::arg))
        .def("arg", py::overload_cast<const py::dict&>(&PyRegisterCallback::arg))
        .def("default_value", py::overload_cast<const py::list&>(&PyRegisterCallback::default_value))
        .def("default_value", py::overload_cast<const py::dict&>(&PyRegisterCallback::default_value))
        .def("callback", py::overload_cast<const py::object&>(&PyRegisterCallback::callback));

    m.def("add_function_to_robot", &addFunctionToRobotPy, "add Function To Robot", "name"_a,
        "callback"_a, "arg_names"_a, "arg_types"_a, "default_value"_a = py::list());

    m.def("add_function_from_robot", &addFunctionFromRobotPy, "add Function From Robot", "name"_a,
        "callback"_a, "return_type"_a);

    m.def("add_value_from_robot", &addValueFromRobotPy, "add Value From Robot", "name"_a, "value"_a,
        "value_type"_a);
    m.def("get_new_function_name", &getNewFunctionName);
}
}  // namespace WebCFace::pybind

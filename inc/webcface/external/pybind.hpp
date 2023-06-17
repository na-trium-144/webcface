#pragma once
#include <webcface/external/pybind/server.hpp>
#include <webcface/external/pybind/registration.hpp>
#include <webcface/external/pybind/logger.hpp>
#include <webcface/external/pybind/layout.hpp>
#include <pybind11/pybind11.h>
namespace WebCFace
{
inline void pybindModuleDef(pybind11::module& m)
{
    pybind::defServer(m);
    pybind::defRegistration(m);
    pybind::defLogger(m);
    pybind::defLayout(m);
}
}  // namespace WebCFace
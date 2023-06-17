#pragma once
#include <webcface/logger.hpp>
namespace WebCFace::pybind
{
inline void defLogger(pybind11::module& m)
{
    using namespace pybind11::literals;
    m.def("init_std_logger", &initStdLogger, "Initialize for Sending cout/cerr (C++ side only) to WebCFace");
    m.def("append_log_line", &appendLogLine, "Append Log by 1 Line", "level"_a, "text"_a);
}
}  // namespace WebCFace::pybind

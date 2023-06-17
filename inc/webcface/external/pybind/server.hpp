#pragma once
#include <webcface/server.hpp>
namespace WebCFace::pybind
{
inline void defServer(pybind11::module& m)
{
    using namespace pybind11::literals;
    m.def(
        "start_server", &startServer, "Start WebCFace Server", "port"_a = 80, "static_dir"_a = "");
    m.def("quit_server", &quitServer, "Quit WebCFace Server");
    m.def("send_data", []{
        pybind11::gil_scoped_release release;
        sendData();
    }, "Send Data to Client");
    m.def("set_server_name", &setServerName, "Set Server Name", "name"_a);
    m.def("add_related_server", pybind11::overload_cast<int>(&addRelatedServer),
        "Add Related Server Address", "port"_a);
    m.def("add_related_server", pybind11::overload_cast<const std::string&, int>(&addRelatedServer),
        "Add Related Server Address", "addr"_a, "port"_a);
    m.def("client_count", &clientCount);
}
}  // namespace WebCFace::pybind

#pragma once
#include <drogon/utils/Utilities.h>
#include <webcface/registration.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <string>
#include <vector>

namespace WebCFace::pybind
{

inline void addImage(
    const std::string& name, const std::string& mime, const pybind11::array_t<uint8_t>& data)
{
    std::string encoded = "";
    const auto& buff_info = data.request();
    const auto& shape = buff_info.shape;
    std::vector<uint8_t> buf(shape[0]);
    for (auto i = 0; i < shape[0]; i++) {
        buf[i] = *data.data(i);
    }
    encoded = "data:" + mime + ";base64," + drogon::utils::base64Encode(buf.data(), buf.size());
    WebCFace::addImage(name, encoded);
}
inline void defImage(pybind11::module& m)
{
    using namespace pybind11::literals;
    m.def("add_image",
        pybind11::overload_cast<const std::string&, const std::string&,
            const pybind11::array_t<uint8_t>&>(&addImage),
        "Add Encoded Image", "name"_a, "mime"_a, "data"_a);
}

}  // namespace WebCFace::pybind

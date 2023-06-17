#ifdef PYWEBCFACE_MODULE_BUILD
// webcfaceライブラリとしてはこのファイルはコンパイルしない
// pywebcfaceのモジュールをビルドするときにこのファイルだけをコンパイル
#include <pybind11/pybind11.h>
#include <webcface/external/pybind.hpp>

namespace py = pybind11;

PYBIND11_MODULE(pywebcface, m)
{
    m.doc() = "WebCFace Plugin";  // optional module docstring
    WebCFace::pybindModuleDef(m);
}
#endif

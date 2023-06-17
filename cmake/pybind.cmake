add_subdirectory(external/pybind11)

set(PY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/pybind/main.cpp)
add_library(pywebcface MODULE ${PY_SOURCES})
target_compile_definitions(pywebcface PRIVATE PYWEBCFACE_MODULE_BUILD)
target_link_libraries(pywebcface PRIVATE
    pybind11::module
    pybind11::lto
    pybind11::windows_extras
    webcface
)

pybind11_extension(pywebcface)
if(NOT MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES Debug|RelWithDebInfo)
    # Strip unnecessary sections of the binary on Linux/macOS
    pybind11_strip(pywebcface)
endif()

set_target_properties(pywebcface PROPERTIES
    CXX_VISIBILITY_PRESET "hidden"
    CUDA_VISIBILITY_PRESET "hidden"
    INSTALL_RPATH "$ORIGIN/;$ORIGIN/../../../"
    # .local/lib/python3.10/site-packages/webcface/ → .local/lib/libwebcface.so
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

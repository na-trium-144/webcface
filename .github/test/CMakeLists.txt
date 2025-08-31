cmake_minimum_required(VERSION 3.12)
if(NOT CMAKE_VERSION VERSION_LESS 3.28)
    cmake_policy(SET CMP0155 NEW)
endif()

project(a)
find_package(webcface REQUIRED)

add_executable(test-cpp test.cc)
target_link_libraries(test-cpp PRIVATE webcface::webcface)

add_executable(test-c test.c)
target_link_libraries(test-c PRIVATE webcface::wcf)

if(
    (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
        NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14) OR
    (CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND
        NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 16) OR
    (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND
        NOT CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14.34)
)
    add_executable(test-module-cpp test_module.cc)
    target_link_libraries(test-module-cpp PRIVATE webcface::webcface_modules)
endif()

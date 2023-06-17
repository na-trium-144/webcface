cmake_minimum_required(VERSION 3.12)
project(webcface_example)
set(WEBCFACE_LEGACY false)
if("${SUBDIR}")
    add_subdirectory(webcface)
else()
    find_package(webcface)
endif()
set(CMAKE_CXX_STANDARD 20)
file(GLOB SRC ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
add_executable(example ${SRC})

webcface_generate(TARGET example_shell NAMESPACE Functions SOURCES ${SRC})

target_link_libraries(example webcface::webcface example_shell)


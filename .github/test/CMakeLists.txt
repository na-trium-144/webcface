cmake_minimum_required(VERSION 3.12)
project(a)
find_package(webcface REQUIRED)

add_executable(test-cpp test.cc)
target_link_libraries(test-cpp PRIVATE webcface::webcface)

add_executable(test-c test.c)
target_link_libraries(test-c PRIVATE webcface::wcf)

cmake_minimum_required(VERSION 3.5)
project(webcface CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

aux_source_directory(src SRC)
aux_source_directory(../server_cpp/message MESSAGE_SRC)
add_library(${PROJECT_NAME} SHARED ${SRC} ${MESSAGE_SRC})
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

# c++20以上が必要(concepts)
set(CMAKE_CXX_STANDARD 20)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${DROGON}
    ${MSGPACK}
)


target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(test_client main.cc)
target_link_libraries(test_client PRIVATE ${PROJECT_NAME})

# add_subdirectory(test)

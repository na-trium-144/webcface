cmake_minimum_required(VERSION 3.5)
project(webcface CXX)

add_compile_options(-g)

# c++20 for concepts
set(CMAKE_CXX_STANDARD 20)


find_package(msgpack CONFIG)
if(NOT ${msgpack_FOUND})
    set(MSGPACK_BUILD_DOCS off CACHE INTERNAL "" FORCE)
    set(MSGPACK_BUILD_TESTS off CACHE INTERNAL "" FORCE)
    set(MSGPACK_BUILD_EXAMPLES off CACHE INTERNAL "" FORCE)
    set(MSGPACK_USE_BOOST off CACHE INTERNAL "" FORCE)
    add_subdirectory(external/msgpack-c)
endif()
set(MSGPACK msgpack-cxx)

find_package(eventpp)
if(NOT ${eventpp_FOUND})
    add_subdirectory(external/eventpp)
endif()
set(EVENTPP eventpp::eventpp)

find_package(spdlog)
if(${spdlog_FOUND})
    set(SPDLOG spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
else()
    add_subdirectory(external/spdlog)
    set(SPDLOG spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endif()

add_library(cinatra INTERFACE)
target_include_directories(cinatra INTERFACE ${CMAKE_CURRENT_LIST_DIR}/external/cinatra/include)
target_compile_options(cinatra INTERFACE -pthread)
target_compile_options(cinatra INTERFACE -fcoroutines -fno-tree-slp-vectorize)


enable_testing() # todo: オプションで変えられるようにする

aux_source_directory(cpp SVR_MAIN_SRC)
aux_source_directory(cpp/server SVR_SRC)
aux_source_directory(cpp/client CLI_SRC)
aux_source_directory(cpp/example EXAMPLE_SRC)
aux_source_directory(cpp/test TEST_SRC)
aux_source_directory(cpp/message MSG_SRC)
aux_source_directory(cpp/controllers CTL_SRC)
# aux_source_directory(cpp/filters FILTER_SRC)
# aux_source_directory(cpp/plugins PLUGIN_SRC)
# aux_source_directory(cpp/models MODEL_SRC)
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include)

###############################################################################
# server

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME}_message STATIC ${MSG_SRC})
target_compile_options(${PROJECT_NAME}_message PRIVATE -fPIC)
target_compile_features(${PROJECT_NAME}_message PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME}_message PUBLIC ${MSGPACK})
target_include_directories(${PROJECT_NAME}_message PUBLIC ${INCLUDE_DIR})

add_library(${PROJECT_NAME}_server_impl STATIC ${SVR_SRC} ${CTL_SRC})
target_link_libraries(${PROJECT_NAME}_server_impl PUBLIC ${PROJECT_NAME}_message cinatra)

add_executable(${PROJECT_NAME}_server ${SVR_MAIN_SRC})
target_link_libraries(${PROJECT_NAME}_server PRIVATE ${PROJECT_NAME}_server_impl)

add_library(${PROJECT_NAME} SHARED ${CLI_SRC})
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_message cinatra)
target_link_libraries(${PROJECT_NAME} PUBLIC ${EVENTPP} ${SPDLOG})
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

foreach(SRC IN LISTS EXAMPLE_SRC)
    get_filename_component(TARGET ${SRC} NAME_WE)
    add_executable(example_${TARGET} ${SRC})
    target_link_libraries(example_${TARGET} PRIVATE ${PROJECT_NAME})
endforeach()

# add_executable(${PROJECT_NAME}_test ${TEST_SRC})
# target_link_libraries(${PROJECT_NAME}_test PRIVATE ${PROJECT_NAME} ${PROJECT_NAME}_server_impl)
# ParseAndAddDrogonTests(${PROJECT_NAME}_test)


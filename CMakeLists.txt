cmake_minimum_required(VERSION 3.9)
project(webcface CXX)

# c++20 for concepts
set(CMAKE_CXX_STANDARD 20)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        add_link_options(-lgcov --coverage)
    else()
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    endif()
endif()


find_package(msgpack-cxx QUIET)
if(${msgpack-cxx_FOUND})
    message("msgpack-cxx found")
else()
    message("msgpack-cxx not found")
    set(MSGPACK_BUILD_DOCS off CACHE INTERNAL "" FORCE)
    set(MSGPACK_BUILD_TESTS off CACHE INTERNAL "" FORCE)
    set(MSGPACK_BUILD_EXAMPLES off CACHE INTERNAL "" FORCE)
    set(MSGPACK_USE_BOOST off CACHE INTERNAL "" FORCE)
    add_subdirectory(external/msgpack-c)
endif()
set(MSGPACK msgpack-cxx)

find_package(eventpp QUIET)
if(${eventpp_FOUND})
    message("eventpp found")
else()
    message("eventpp not found")
    add_subdirectory(external/eventpp)
endif()
set(EVENTPP eventpp::eventpp)

find_package(spdlog QUIET)
if(${spdlog_FOUND})
    message("spdlog found")
    set(SPDLOG spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
else()
    message("spdlog not found")
    set(SPDLOG_BUILD_PIC on CACHE INTERNAL "" FORCE)
    add_subdirectory(external/spdlog)
    set(SPDLOG spdlog $<$<BOOL:${MINGW}>:ws2_32>)
endif()

add_library(cinatra INTERFACE)
target_include_directories(cinatra INTERFACE ${CMAKE_CURRENT_LIST_DIR}/external/cinatra/include)
target_compile_options(cinatra INTERFACE -pthread)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(cinatra INTERFACE -fcoroutines -fno-tree-slp-vectorize)
endif()

add_library(tclap INTERFACE)
target_include_directories(tclap INTERFACE ${CMAKE_CURRENT_LIST_DIR}/external/tclap/include)

find_package(GTest QUIET)
if(${GTest_FOUND})
    message("GTest found")
else()
    message("GTest not found")
    add_subdirectory(external/googletest)
endif()

set(CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SVR_MAIN_SRC ${CPP_DIR}/server_main.cc)
set(SVR_SRC
    ${CPP_DIR}/server/s_client_data.cc
    ${CPP_DIR}/server/store.cc
    ${CPP_DIR}/server/websock.cc
    ${CPP_DIR}/server/dir.cc
)
set(CLI_SRC
    ${CPP_DIR}/client/client.cc
    ${CPP_DIR}/client/client_data.cc
    ${CPP_DIR}/client/client_ws.cc
    ${CPP_DIR}/client/field.cc
    ${CPP_DIR}/client/func.cc
    ${CPP_DIR}/client/logger.cc
    ${CPP_DIR}/client/member.cc
)
set(EXAMPLE_SRC
    ${CPP_DIR}/example/main.cc
    ${CPP_DIR}/example/recv.cc
)
set(TEST_SRC
    ${CPP_DIR}/test/client_data_test.cc
    ${CPP_DIR}/test/data_test.cc
    ${CPP_DIR}/test/func_test.cc
    ${CPP_DIR}/test/view_test.cc
    ${CPP_DIR}/test/member_test.cc
    ${CPP_DIR}/test/logger_test.cc
    ${CPP_DIR}/test/client_test.cc
    ${CPP_DIR}/test/dummy_server.cc
    ${CPP_DIR}/test/server_test.cc
)
set(MSG_SRC
    ${CPP_DIR}/message/message.cc
)
set(INCLUDE_DIR ${CPP_DIR}/include)

###############################################################################
# server

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(${PROJECT_NAME}_message STATIC ${MSG_SRC})
target_compile_options(${PROJECT_NAME}_message PRIVATE -fPIC)
target_compile_features(${PROJECT_NAME}_message PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME}_message PUBLIC
    ${MSGPACK}
    ${SPDLOG}
)
target_include_directories(${PROJECT_NAME}_message PUBLIC ${INCLUDE_DIR})

add_library(${PROJECT_NAME}_server_impl STATIC ${SVR_SRC})
target_link_libraries(${PROJECT_NAME}_server_impl PUBLIC
    ${PROJECT_NAME}_message
    cinatra
    ${SPDLOG}
)

add_executable(${PROJECT_NAME}_server ${SVR_MAIN_SRC})
target_link_libraries(${PROJECT_NAME}_server PRIVATE
    ${PROJECT_NAME}_server_impl
    tclap
)

add_library(${PROJECT_NAME} SHARED ${CLI_SRC})
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}_message
    cinatra
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${EVENTPP}
    ${SPDLOG}
)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})

foreach(SRC IN LISTS EXAMPLE_SRC)
    get_filename_component(TARGET ${SRC} NAME_WE)
    add_executable(example_${TARGET} ${SRC})
    target_link_libraries(example_${TARGET} PRIVATE ${PROJECT_NAME})
endforeach()


enable_testing() # todo: オプションで変えられるようにする

add_executable(${PROJECT_NAME}_test ${TEST_SRC})
target_link_libraries(${PROJECT_NAME}_test PRIVATE
    ${PROJECT_NAME}
    ${PROJECT_NAME}_message
    ${PROJECT_NAME}_server_impl
    cinatra
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME}_test)

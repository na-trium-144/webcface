cmake_minimum_required(VERSION 3.9)
project(webcface
    LANGUAGES CXX
    VERSION 1.0.0
    HOMEPAGE_URL "https://github.com/na-trium-144/webcface"
    DESCRIPTION "Web-based RPC & UI Library"
)
set(WEBCFACE_SOVERSION 1)

if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
    set(IS_MAIN on)
else()
    set(IS_MAIN off)
endif()

option(WEBCFACE_EXAMPLE "build example" OFF)
option(WEBCFACE_TEST "build test" OFF)
option(WEBCFACE_INSTALL "generate install target" ${IS_MAIN})

message(STATUS "webcface_example = ${WEBCFACE_EXAMPLE}")
message(STATUS "webcface_test = ${WEBCFACE_TEST}")
message(STATUS "webcface_install = ${WEBCFACE_INSTALL}")

# c++20 for concepts
# set(CMAKE_CXX_STANDARD 20)
# set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_EXTENSIONS OFF)

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
    message(STATUS "msgpack-cxx found")
else()
    message(STATUS "msgpack-cxx not found")
    # set(MSGPACK_BUILD_DOCS off CACHE INTERNAL "" FORCE)
    # set(MSGPACK_BUILD_TESTS off CACHE INTERNAL "" FORCE)
    # set(MSGPACK_BUILD_EXAMPLES off CACHE INTERNAL "" FORCE)
    # set(MSGPACK_USE_BOOST off CACHE INTERNAL "" FORCE)
    # add_subdirectory(external/msgpack-c)
    # このようにmsgpackのcmakeを使うと、msgpackをインストールしてしまう

    add_library(msgpack-cxx INTERFACE)
    target_compile_definitions(msgpack-cxx INTERFACE
        MSGPACK_NO_BOOST
        MSGPACK_DEFAULT_API_VERSION=3
    )
    target_include_directories(msgpack-cxx INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/external/msgpack-c/include
    )
endif()
set(MSGPACK msgpack-cxx)

find_package(eventpp QUIET)
if(${eventpp_FOUND})
    message(STATUS "eventpp found")
else()
    message(STATUS "eventpp not found")
    set(EVENTPP_INSTALL ${WEBCFACE_INSTALL} CACHE INTERNAL "" FORCE)
    add_subdirectory(external/eventpp)
endif()
set(EVENTPP eventpp::eventpp)

find_package(spdlog QUIET)
if(${spdlog_FOUND})
    message(STATUS "spdlog found")
    set(SPDLOG spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)
else()
    message(STATUS "spdlog not found")
    set(SPDLOG_BUILD_SHARED on CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_PIC on CACHE INTERNAL "" FORCE)
    set(SPDLOG_INSTALL ${WEBCFACE_INSTALL} CACHE INTERNAL "" FORCE)
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

if(${WEBCFACE_TEST})
    find_package(GTest QUIET)
    if(${GTest_FOUND})
        message(STATUS "GTest found")
    else()
        message(STATUS "GTest not found")
        add_subdirectory(external/googletest)
    endif()
endif()

###############################################################################

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
set(INCLUDE_DIR
    $<BUILD_INTERFACE:${CPP_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

###############################################################################

add_library(${PROJECT_NAME}-message STATIC ${MSG_SRC})
target_compile_options(${PROJECT_NAME}-message PRIVATE -fPIC)
target_compile_features(${PROJECT_NAME}-message PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME}-message PUBLIC
    ${MSGPACK}
    ${SPDLOG}
)
target_include_directories(${PROJECT_NAME}-message PUBLIC ${INCLUDE_DIR})

add_library(${PROJECT_NAME}-server-impl STATIC ${SVR_SRC})
target_link_libraries(${PROJECT_NAME}-server-impl PUBLIC
    ${PROJECT_NAME}-message
    cinatra
    ${SPDLOG}
)

add_executable(${PROJECT_NAME}-server ${SVR_MAIN_SRC})
target_link_libraries(${PROJECT_NAME}-server PRIVATE
    ${PROJECT_NAME}-server-impl
    tclap
)

add_library(${PROJECT_NAME} SHARED ${CLI_SRC})
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}-message
    cinatra
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${EVENTPP}
    ${SPDLOG}
)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/;"
    INSTALL_RPATH_USE_LINK_PATH TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${WEBCFACE_SOVERSION}
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


if(${WEBCFACE_EXAMPLE})
    foreach(SRC IN LISTS EXAMPLE_SRC)
        get_filename_component(TARGET ${SRC} NAME_WE)
        add_executable(example_${TARGET} ${SRC})
        target_link_libraries(example_${TARGET} PRIVATE ${PROJECT_NAME})
    endforeach()
endif()

if(${WEBCFACE_TEST})
    enable_testing()
    add_executable(${PROJECT_NAME}-test ${TEST_SRC})
    target_link_libraries(${PROJECT_NAME}-test PRIVATE
        ${PROJECT_NAME}
        ${PROJECT_NAME}-message
        ${PROJECT_NAME}-server-impl
        cinatra
        GTest::gtest_main
    )
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}-test)
endif()


###############################################################################
# https://qiita.com/modapone/items/8f97425b6167cffc815c
if(${WEBCFACE_INSTALL})
    include(CMakePackageConfigHelpers)

    install(TARGETS ${PROJECT_NAME}-server
        RUNTIME DESTINATION bin
    )
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION lib
    )
    # インストールするconfigの指定、前項で生成したもの
    install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        # 名前空間の指定
        NAMESPACE ${PROJECT_NAME}::
        # インストール先
        DESTINATION lib/cmake/${PROJECT_NAME}
    )
    # コピー元、末尾の`/`は要注意
    install(DIRECTORY ${CPP_DIR}/include/
        DESTINATION include
    )
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    # Package Version Fileの生成
    write_basic_package_version_file(
        # 生成するファイルの名前
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        # 互換性の判定方法の指定
        COMPATIBILITY SameMajorVersion
    )

    # Package Version Fileのインストール
    # コピー元
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        # コピー先
        DESTINATION lib/cmake/${PROJECT_NAME}
    )

    set(CPACK_PACKAGE_CONTACT "na-trium-144")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6")
    include(CPack)
endif()

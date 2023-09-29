cmake_minimum_required(VERSION 3.9)

# src/include/webcface/common/def.h からバージョンを取得
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/src/include/webcface/common/def.h contents)
string(REGEX MATCH "#define WEBCFACE_VERSION_MAJOR *([0-9a-zA-Z_]*)" NULL_OUT ${contents})
set(VERSION_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define WEBCFACE_VERSION_MINOR *([0-9a-zA-Z_]*)" NULL_OUT ${contents})
set(VERSION_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define WEBCFACE_VERSION_REVISION *([0-9a-zA-Z_]*)" NULL_OUT ${contents})
set(VERSION_REVISION ${CMAKE_MATCH_1})
set(VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION})

project(webcface
    LANGUAGES CXX
    VERSION ${VERSION}
    HOMEPAGE_URL "https://github.com/na-trium-144/webcface"
    DESCRIPTION "Web-based RPC & UI Library"
)
set(WEBCFACE_SOVERSION 1)
message(STATUS "webcface ${PROJECT_VERSION}")

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

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-psabi)
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
set(CINATRA cinatra $<$<BOOL:${MINGW}>:ws2_32 wsock32>)

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
include_directories(${INCLUDE_DIR})

###############################################################################

add_library(${PROJECT_NAME}-message STATIC ${MSG_SRC})
target_compile_options(${PROJECT_NAME}-message PRIVATE -fPIC)
target_compile_features(${PROJECT_NAME}-message PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME}-message PUBLIC
    ${MSGPACK}
    ${SPDLOG}
)

add_library(${PROJECT_NAME}-server-impl STATIC ${SVR_SRC})
target_link_libraries(${PROJECT_NAME}-server-impl PUBLIC
    ${PROJECT_NAME}-message
    ${CINATRA}
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
    ${CINATRA}
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
        ${CINATRA}
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

    # https://decovar.dev/blog/2021/09/23/cmake-cpack-package-deb-apt/
    set(CPACK_VERBATIM_VARIABLES yes)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_PACKAGE_CONTACT "nakak_u21@yahoo.co.jp")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "na-trium-144 <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS yes)
    if(NOT ${spdlog_FOUND})
        # submoduleのspdlogを使った場合システムのspdlogとはconflictする
        set(CPACK_DEBIAN_PACKAGE_CONFLICTS "libspdlog-dev")
    endif()
    set(CPACK_STRIP_FILES yes)
    set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
    # https://gitlab.kitware.com/cmake/cmake/-/issues/21834
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_LIST_DIR}/triggers")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)

    include(CPack)
endif()

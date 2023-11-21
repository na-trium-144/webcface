cmake_minimum_required(VERSION 3.12)

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

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IS_MAIN on)
else()
    set(IS_MAIN off)
endif()

option(WEBCFACE_INSTALL "generate install target" ${IS_MAIN})
option(WEBCFACE_EXAMPLE "build example" ${IS_MAIN})
option(WEBCFACE_TEST "build test" OFF)
option(WEBCFACE_COVERAGE "enable generating test coverage" OFF)
set(WEBCFACE_TEST_TIMEOUT 10 CACHE INTERNAL "connection timeout in test")
option(WEBCFACE_SHARED "build shared library" ON)
option(WEBCFACE_PIC "enable PIC for shared library" ${WEBCFACE_SHARED})

message(STATUS "webcface_example = ${WEBCFACE_EXAMPLE}")
message(STATUS "webcface_test = ${WEBCFACE_TEST} (timeout = ${WEBCFACE_TEST_TIMEOUT} ms)")
message(STATUS "webcface_coverage = ${WEBCFACE_COVERAGE}")
message(STATUS "webcface_install = ${WEBCFACE_INSTALL}")
message(STATUS "webcface_shared = ${WEBCFACE_SHARED} (pic = ${WEBCFACE_PIC})")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(WEBCFACE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        add_link_options(-lgcov --coverage)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    endif()
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-psabi)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(-utf-8)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(gtest_force_shared_crt on)
endif()

if(MINGW)
    # https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean
    add_link_options(-Wl,--default-image-base-low)
endif()

find_package(msgpack-cxx QUIET)
if(msgpack-cxx_FOUND)
    message(STATUS "msgpack-cxx found")
else()
    message(STATUS "msgpack-cxx not found")
    # msgpackのcmakelistsを使うとmsgpackをインストールしてしまうので、includeするだけ

    add_library(msgpack-cxx INTERFACE)
    target_compile_definitions(msgpack-cxx INTERFACE
        MSGPACK_NO_BOOST
        MSGPACK_DEFAULT_API_VERSION=3
    )
    target_include_directories(msgpack-cxx INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/external/msgpack-c/include>
        $<INSTALL_INTERFACE:include>
    )
endif()
set(MSGPACK msgpack-cxx)

find_package(eventpp QUIET)
if(eventpp_FOUND)
    message(STATUS "eventpp found")
else()
    message(STATUS "eventpp not found")
    set(EVENTPP_INSTALL ${WEBCFACE_INSTALL} CACHE INTERNAL "" FORCE)
    add_subdirectory(external/eventpp)
endif()
set(EVENTPP eventpp::eventpp)

find_package(spdlog QUIET)
if(spdlog_FOUND)
    message(STATUS "spdlog found")
    set(SPDLOG spdlog::spdlog)
else()
    message(STATUS "spdlog not found")
    set(SPDLOG_BUILD_SHARED ${WEBCFACE_SHARED} CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_PIC ${WEBCFACE_PIC} CACHE INTERNAL "" FORCE)
    set(SPDLOG_INSTALL ${WEBCFACE_INSTALL} CACHE INTERNAL "" FORCE)
    add_subdirectory(external/spdlog)
    if(MINGW)
        set(SPDLOG spdlog ws2_32)
    else()
        set(SPDLOG spdlog)
    endif()
endif()

set(CURL_ENABLE_SSL off CACHE INTERNAL "" FORCE)
set(BUILD_CURL_EXE off CACHE INTERNAL "" FORCE)
# libcurlはだいたいインストールされているが使えないのでsharedにする意味がない
set(BUILD_SHARED_LIBS off CACHE INTERNAL "" FORCE)
set(BUILD_STATIC_LIBS on CACHE INTERNAL "" FORCE)
set(SHARE_LIB_OBJECT ${WEBCFACE_PIC} CACHE INTERNAL "" FORCE)
set(ENABLE_WEBSOCKETS on CACHE INTERNAL "" FORCE)
set(CURL_DISABLE_INSTALL on CACHE INTERNAL "" FORCE)
add_subdirectory(external/curl)
set(CURL libcurl_static)

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE ${CMAKE_CURRENT_LIST_DIR}/external/asio/asio/include)
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
if(MINGW)
    set(ASIO asio ws2_32 wsock32)
else()
    set(ASIO asio)
endif()

# asioがない場合find_packageしないようにするため、
# add_subdirectoryではなくinterfaceライブラリを定義する
add_library(Crow INTERFACE)
target_include_directories(Crow INTERFACE ${CMAKE_CURRENT_LIST_DIR}/external/crow/include)
target_link_libraries(Crow INTERFACE ${ASIO})
set(CROW Crow)

add_subdirectory(external/cli11)

if(WEBCFACE_TEST)
    find_package(GTest QUIET)
    if(GTest_FOUND)
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
    ${CPP_DIR}/client/client_ws.cc
    ${CPP_DIR}/client/data_store1.cc
    ${CPP_DIR}/client/data_store2.cc
    ${CPP_DIR}/client/field.cc
    ${CPP_DIR}/client/func.cc
    ${CPP_DIR}/client/log.cc
    ${CPP_DIR}/client/logger.cc
    ${CPP_DIR}/client/member.cc
    ${CPP_DIR}/client/text.cc
    ${CPP_DIR}/client/value.cc
    ${CPP_DIR}/client/view.cc
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
    ${CPP_DIR}/test/dummy_client.cc
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
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND WEBCFACE_PIC)
    target_compile_options(${PROJECT_NAME}-message PRIVATE -fPIC)
endif()
target_compile_features(${PROJECT_NAME}-message PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME}-message PUBLIC
    ${MSGPACK}
    ${SPDLOG}
)

add_library(${PROJECT_NAME}-server-impl STATIC ${SVR_SRC})
target_link_libraries(${PROJECT_NAME}-server-impl PUBLIC
    ${PROJECT_NAME}-message
    ${CROW}
    ${SPDLOG}
)

add_executable(${PROJECT_NAME}-server ${SVR_MAIN_SRC})
target_link_libraries(${PROJECT_NAME}-server PRIVATE
    ${PROJECT_NAME}-server-impl
    CLI11
)
set_target_properties(${PROJECT_NAME}-server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

if(WEBCFACE_SHARED)
    add_library(${PROJECT_NAME} SHARED ${CLI_SRC})
else()
    add_library(${PROJECT_NAME} STATIC ${CLI_SRC})
endif()
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND WEBCFACE_PIC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}-message
    ${CURL}
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${EVENTPP}
    ${SPDLOG}
)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${WEBCFACE_SOVERSION}
)

add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})


if(WEBCFACE_EXAMPLE)
    foreach(SRC IN LISTS EXAMPLE_SRC)
        get_filename_component(TARGET ${SRC} NAME_WE)
        add_executable(${PROJECT_NAME}-example-${TARGET} ${SRC})
        target_link_libraries(${PROJECT_NAME}-example-${TARGET} PRIVATE ${PROJECT_NAME})
        set(EXAMPLE_TARGETS ${EXAMPLE_TARGETS} ${PROJECT_NAME}-example-${TARGET})
        set_target_properties(${PROJECT_NAME}-example-${TARGET} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
    endforeach()
endif()

if(WEBCFACE_TEST)
    enable_testing()
    add_executable(${PROJECT_NAME}-test ${TEST_SRC})
    target_link_libraries(${PROJECT_NAME}-test PRIVATE
        ${PROJECT_NAME}
        ${PROJECT_NAME}-message
        ${PROJECT_NAME}-server-impl
        ${CURL}
        ${CROW}
        GTest::gtest_main
    )
    target_compile_definitions(${PROJECT_NAME}-test PRIVATE
        WEBCFACE_TEST_TIMEOUT=${WEBCFACE_TEST_TIMEOUT}
    )
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}-test)
endif()


###############################################################################
# https://qiita.com/modapone/items/8f97425b6167cffc815c
if(WEBCFACE_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS ${PROJECT_NAME}-server ${EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
    )
    install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION lib
    )
    if(NOT WEBCFACE_SHARED)
        set(WEBCFACE_EXPORTS ${PROJECT_NAME}-message libcurl_static)
        if(NOT msgpack-cxx_FOUND)
            set(WEBCFACE_EXPORTS ${WEBCFACE_EXPORTS} msgpack-cxx)
        endif()
        install(TARGETS ${WEBCFACE_EXPORTS} EXPORT ${PROJECT_NAME}-targets)
    endif()

    install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        # 名前空間の指定
        NAMESPACE ${PROJECT_NAME}::
        # インストール先
        DESTINATION lib/cmake/${PROJECT_NAME}
    )
    install(DIRECTORY ${CPP_DIR}/include/
        DESTINATION include
    )
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/${PROJECT_NAME}-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        DESTINATION lib/cmake/${PROJECT_NAME}
    )

    # https://decovar.dev/blog/2021/09/23/cmake-cpack-package-deb-apt/
    set(CPACK_VERBATIM_VARIABLES yes)
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_PACKAGE_CONTACT "nakak_u21@yahoo.co.jp")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "na-trium-144 <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS yes)
    if(${spdlog_FOUND})
        # submoduleのspdlogを使った場合システムのspdlogとはconflictする
        set(CPACK_DEBIAN_PACKAGE_DEPENDS "libspdlog-dev")
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

cmake_minimum_required(VERSION 3.12)
# ubuntu20.04: cmake 3.16.3

set(ORIGINAL_ENV_CC "$ENV{CC}" CACHE STRING "")
set(ORIGINAL_ENV_CXX "$ENV{CXX}" CACHE STRING "")

project(webcface
    LANGUAGES CXX C
    VERSION 1.11.3
    HOMEPAGE_URL "https://github.com/na-trium-144/webcface"
    DESCRIPTION "Web-based RPC & UI Library"
)
set(WEBCFACE_CONTACT "na-trium-144@users.noreply.github.com")
set(WEBCFACE_VERSION ${PROJECT_VERSION})
set(WEBCFACE_SOVERSION 12)
set(WEBUI_VERSION 1.6.0)

set(WEBCFACE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(WEBCFACE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(WEBCFACE_VERSION_REVISION ${PROJECT_VERSION_PATCH})

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IS_MAIN on)
else()
    set(IS_MAIN off)
endif()

option(WEBCFACE_INSTALL "generate install target" ${IS_MAIN})
option(WEBCFACE_INSTALL_SERVICE "generate install target for systemd service file" OFF)
option(WEBCFACE_EXAMPLE "build example" ${IS_MAIN})
option(WEBCFACE_TEST "build test" OFF)
option(WEBCFACE_COVERAGE "enable generating test coverage" OFF)
set(WEBCFACE_TEST_TIMEOUT 10 CACHE STRING "connection timeout in test")
option(WEBCFACE_SHARED "build shared library" ON)
option(WEBCFACE_PIC "enable PIC for shared library" ${WEBCFACE_SHARED})
set(WEBCFACE_VERSION_SUFFIX "git" CACHE STRING "version suffix ('git' to get automatically from git describe command)")
set(WEBCFACE_CLANG_TIDY "" CACHE STRING "clang-tidy path (or empty to disable clang-tidy)")
option(WEBCFACE_DOWNLOAD_WEBUI "download and install webui" ON)
option(WEBCFACE_CONFIG_ALL "configure both debug and release build on windows" OFF)
option(WEBCFACE_FIND_LIBS "if false, disables every find_package and pkg_check_modules" ON)
option(WEBCFACE_FIND_MSGPACK "try find_package(msgpack-cxx)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_SPDLOG "try find_package(spdlog)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_EVENTPP "try find_package(eventpp)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_CURL "try find_package(CURL)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_ASIO "try find_path(asio.hpp)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_CROW "try find_package(Crow)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_CLI11 "try find_package(CLI11)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_UTF8CPP "try find_path(utf8.h)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_MAGICK "try pkg_check_modules(Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_JPEG "try pkg_check_modules(libjpeg) (while building Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_PNG "try pkg_check_modules(libpng) (while building Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_ZLIB "try find_package(zlib) (while building libpng)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_WEBP "try pkg_check_modules(libwebp) (while building Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_AMENT "try find_package(ament_cmake)" ON)
option(WEBCFACE_FIND_OPENCV "try find_package(OpenCV) (only for example)" OFF)

if(WEBCFACE_VERSION_SUFFIX STREQUAL "git")
    execute_process(
        COMMAND git describe --tags
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE git_describe
        ERROR_VARIABLE git_describe_error
        RESULT_VARIABLE git_describe_ok
    )
    if(git_describe_ok STREQUAL 0)
        string(REGEX MATCH "([-0-9a-g\\.]+)" NULL_OUT ${git_describe})
        set(WEBCFACE_VERSION ${CMAKE_MATCH_1})
    else()
        message(STATUS "Failed to get version from git describe: ${git_describe_error}")
    endif()
elseif(NOT WEBCFACE_VERSION_SUFFIX STREQUAL "")
    set(WEBCFACE_VERSION "${PROJECT_VERSION}-${WEBCFACE_VERSION_SUFFIX}")
endif()

if(WEBCFACE_PIC OR CMAKE_POSITION_INDEPENDENT_CODE)
    set(WEBCFACE_PIC ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "")
endif()

message(STATUS "WebCFace ${WEBCFACE_VERSION} (soversion ${WEBCFACE_SOVERSION})")

message(STATUS "webcface_example = ${WEBCFACE_EXAMPLE}")
message(STATUS "webcface_test = ${WEBCFACE_TEST} (timeout = ${WEBCFACE_TEST_TIMEOUT} ms)")
message(STATUS "webcface_coverage = ${WEBCFACE_COVERAGE}")
message(STATUS "webcface_install = ${WEBCFACE_INSTALL}")
message(STATUS "webcface_install_service = ${WEBCFACE_INSTALL_SERVICE}")
message(STATUS "webcface_shared = ${WEBCFACE_SHARED} (pic = ${WEBCFACE_PIC})")
message(STATUS "webcface_config_all = ${WEBCFACE_CONFIG_ALL}")
message(STATUS "webcface_find_msgpack = ${WEBCFACE_FIND_MSGPACK}")
message(STATUS "webcface_find_spdlog = ${WEBCFACE_FIND_SPDLOG}")
message(STATUS "webcface_find_eventpp = ${WEBCFACE_FIND_EVENTPP}")
message(STATUS "webcface_find_magick = ${WEBCFACE_FIND_MAGICK}")
message(STATUS "webcface_find_jpeg = ${WEBCFACE_FIND_JPEG}")
message(STATUS "webcface_find_png = ${WEBCFACE_FIND_PNG}")
message(STATUS "webcface_find_zlib = ${WEBCFACE_FIND_ZLIB}")
message(STATUS "webcface_find_webp = ${WEBCFACE_FIND_WEBP}")
message(STATUS "webcface_find_ament = ${WEBCFACE_FIND_AMENT}")
message(STATUS "webcface_find_opencv = ${WEBCFACE_FIND_OPENCV}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()
message(STATUS "cmake_build_type = ${CMAKE_BUILD_TYPE}")

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

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
    )
    set(gtest_force_shared_crt on CACHE INTERNAL "" FORCE)
endif()

if(MINGW)
    # https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean
    add_link_options(-Wl,--default-image-base-low)
endif()

# ver1.11.2以前はwebcface/common/def.hとversion.rcをsrcに書き込んでいたがそれを消す
set(WEBCFACE_DEF_H_PATH include/webcface/common/def.h)
set(WEBCFACE_VERSION_RC_PATH version.rc)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/${WEBCFACE_DEF_H_PATH})
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/src/${WEBCFACE_DEF_H_PATH})
endif()
configure_file(
    cmake/def.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${WEBCFACE_DEF_H_PATH}
    @ONLY
)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/${WEBCFACE_VERSION_RC_PATH})
    file(REMOVE ${CMAKE_CURRENT_SOURCE_DIR}/src/${WEBCFACE_VERSION_RC_PATH})
endif()
configure_file(
    cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/${WEBCFACE_VERSION_RC_PATH}
    @ONLY
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CheckIncludeFileCXX)
check_include_file_cxx(concepts CONCEPTS_FOUND)
if (NOT CONCEPTS_FOUND)
    message(FATAL_ERROR "<concepts> header file not found")
endif()

###############################################################################

include(FetchContent)
include(FindPkgConfig)

if(WEBCFACE_DOWNLOAD_WEBUI)
    message(STATUS "Downloading webcface-webui ${WEBUI_VERSION}")
    FetchContent_Declare(webui
        URL https://github.com/na-trium-144/webcface-webui/releases/download/v${WEBUI_VERSION}/webcface-webui_${WEBUI_VERSION}.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist
    )
    FetchContent_Populate(webui)
endif()

if(WEBCFACE_FIND_AMENT)
    find_package(ament_cmake QUIET)
endif()
if(ament_cmake_FOUND)
    message(STATUS "ament_cmake Found")
endif()

if(WEBCFACE_FIND_MSGPACK)
    find_package(msgpack-cxx QUIET)
endif()
if(msgpack-cxx_FOUND)
    message(STATUS "msgpack-cxx ${msgpack-cxx_VERSION} Found: ${msgpack-cxx_DIR}")
else()
    message(STATUS "msgpack-cxx Not Found, Fetching Source...")
    FetchContent_Declare(msgpack-cxx
        GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
        GIT_TAG cpp-6.1.0
    )
    # msgpackのcmakelistsを使うとmsgpackをインストールしてしまうので、includeするだけ
    FetchContent_GetProperties(msgpack-cxx)
    if(NOT msgpack-cxx_POPULATED)
        FetchContent_Populate(msgpack-cxx)
        add_library(msgpack-cxx INTERFACE)
        target_compile_definitions(msgpack-cxx INTERFACE
            MSGPACK_NO_BOOST
            MSGPACK_DEFAULT_API_VERSION=3
        )
        target_include_directories(msgpack-cxx INTERFACE $<BUILD_INTERFACE:${msgpack-cxx_SOURCE_DIR}/include>)
    endif()
endif()

if(WEBCFACE_FIND_EVENTPP)
    find_package(eventpp QUIET)
endif()
if(eventpp_FOUND)
    message(STATUS "eventpp ${eventpp_VERSION} Found: ${eventpp_DIR}")
else()
    message(STATUS "eventpp Not Found, Fetching Source...")
    FetchContent_Declare(eventpp
        GIT_REPOSITORY https://github.com/wqking/eventpp.git
        GIT_TAG v0.1.3
    )
    set(EVENTPP_INSTALL OFF CACHE INTERNAL "" FORCE)
    FetchContent_MakeAvailable(eventpp)
endif()

if(WEBCFACE_FIND_SPDLOG)
    find_package(spdlog QUIET)
endif()
if(spdlog_FOUND)
    message(STATUS "spdlog ${spdlog_VERSION} Found: ${spdlog_DIR}")
    set(spdlog spdlog::spdlog)
else()
    message(STATUS "spdlog Not Found, Fetching Source...")
    FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    set(SPDLOG_INSTALL OFF CACHE INTERNAL "" FORCE)
    if(WIN32)
        set(SPDLOG_WCHAR_SUPPORT ON CACHE INTERNAL "" FORCE)
    endif()
    FetchContent_MakeAvailable(spdlog)
    set(spdlog spdlog_header_only)
endif()

if(WEBCFACE_FIND_CURL)
    find_package(CURL QUIET)
endif()
if(CURL_FOUND)
    message(STATUS "CURL ${CURL_VERSION_STRING} Found: ${PC_CURL_LIBDIR}")
    include(CheckCXXSourceRuns)
    set(CMAKE_REQUIRED_LIBRARIES CURL::libcurl)
    check_cxx_source_runs("
#include <curl/curl.h>
#include <cassert>
#include <cstddef>
int main() {
    CURL *curl = curl_easy_init();
    std::size_t s;
    assert(curl_ws_send(curl, nullptr, 0, &s, 0, 0) != CURLE_NOT_BUILT_IN);
}" CURL_HAS_WS_SUPPORT)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(NOT CURL_HAS_WS_SUPPORT)
        message(STATUS "Installed curl has no websockets support")
        unset(CURL_FOUND)
    endif()
endif()
if(CURL_FOUND)
    set(libcurl CURL::libcurl)
else()
    message(STATUS "CURL Not Found, Fetching Source...")
    FetchContent_Declare(curl
        GIT_REPOSITORY https://github.com/curl/curl.git
        GIT_TAG curl-8_5_0
    )
    set(CURL_ENABLE_SSL off CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_ALTSVC on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SRP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_BASIC_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_BEARER_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_DIGEST_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_KERBEROS_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_NEGOTIATE_AUTH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_AWS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_DICT on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_DOH on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_FILE on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_FORM_API on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_FTP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_GETOPTIONS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_GOPHER on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_HSTS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_IMAP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_LDAP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_LDAPS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_LIBCURL_OPTION on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_MIME on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_MQTT on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_BINDLOCAL on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_NETRC on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_NTLM on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_PARSEDATE on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_POP3 on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_PROGRESS_METER on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_RTSP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SMB on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SMTP on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_SOCKETPAIR on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_TELNET on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_TFTP on CACHE INTERNAL "" FORCE)
    set(CURL_ZLIB off CACHE INTERNAL "" FORCE)
    set(CURL_USE_LIBSSH2 off CACHE INTERNAL "" FORCE)
    set(CURL_USE_LIBSSH off CACHE INTERNAL "" FORCE)
    set(BUILD_CURL_EXE off CACHE INTERNAL "" FORCE)
    set(BUILD_SHARED_LIBS off CACHE INTERNAL "" FORCE)
    set(BUILD_STATIC_LIBS on CACHE INTERNAL "" FORCE)
    set(SHARE_LIB_OBJECT ${WEBCFACE_PIC} CACHE INTERNAL "" FORCE)
    set(ENABLE_WEBSOCKETS on CACHE INTERNAL "" FORCE)
    set(CURL_DISABLE_INSTALL on CACHE INTERNAL "" FORCE)
    set(CURL_ENABLE_EXPORT_TARGET off CACHE INTERNAL "" FORCE)
    FetchContent_MakeAvailable(curl)
    set(libcurl libcurl_static)
endif()

if(WEBCFACE_FIND_CROW)
    find_package(Crow QUIET)
endif()
if(Crow_FOUND)
    message(STATUS "Crow ${Crow_VERSION} Found: ${Crow_DIR}")
    include(CheckCXXSourceCompiles)
    set(CMAKE_REQUIRED_LIBRARIES Crow::Crow)
    check_cxx_source_compiles("
#include <crow.h>
int main(){
    crow::SimpleApp app;
    CROW_ROUTE(app, \"/\")([](){
        return \"Hello world\";
    });
    app.unix_path(\"/tmp/test.sock\").multithreaded().run();
}" CROW_HAS_UNIX_SUPPORT)
    unset(CMAKE_REQUIRED_LIBRARIES)
    if(NOT CROW_HAS_UNIX_SUPPORT)
        message(STATUS "Installed crow has no unix socket support")
        unset(Crow_FOUND)
    else()
        set(crow Crow::Crow)
    endif()
endif()
if(NOT Crow_FOUND)
    message(STATUS "Crow Not Found")
    if(WEBCFACE_FIND_ASIO)
        find_path(ASIO_INCLUDE_DIR asio.hpp)
    endif()
    if(WEBCFACE_FIND_ASIO AND NOT ASIO_INCLUDE_DIR STREQUAL "ASIO_INCLUDE_DIR-NOTFOUND")
        set(ASIO_FOUND true)
        message(STATUS "Asio Found: ${ASIO_INCLUDE_DIR}")
    else()
        set(ASIO_FOUND false)
        message(STATUS "Asio Not Found, Fetching Source...")
        FetchContent_Declare(asio
            GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
            GIT_TAG asio-1-29-0
        )
        FetchContent_GetProperties(asio)
        if(NOT asio_POPULATED)
            FetchContent_Populate(asio)
            set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE INTERNAL "" FORCE)
        endif()
    endif()

    message(STATUS "Fetching Crow Source...")
    # asioがない場合find_packageしないようにするため、
    # add_subdirectoryではなくinterfaceライブラリを定義する
    FetchContent_Declare(crow
        # GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
        # GIT_TAG 921ce6f
        GIT_REPOSITORY https://github.com/na-trium-144/Crow.git
        GIT_TAG 5f5372ed80860dfcef788972bb0fd3972f715842
    )
    FetchContent_GetProperties(crow)
    if(NOT crow_POPULATED)
        FetchContent_Populate(crow)
        add_library(Crow INTERFACE)
        target_include_directories(Crow INTERFACE
            $<BUILD_INTERFACE:${crow_SOURCE_DIR}/include>
            $<BUILD_INTERFACE:${ASIO_INCLUDE_DIR}>
        )
        set(Crow Crow)
    endif()
endif()

if(WEBCFACE_FIND_CLI11)
    find_package(CLI11 QUIET)
endif()
if(CLI11_FOUND)
    message(STATUS "CLI11 ${CLI11_VERSION} Found: ${CLI11_DIR}")
else()
    message(STATUS "CLI11 Not Found, Fetching Source...")
    FetchContent_Declare(cli11
        GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
        GIT_TAG v2.3.2
    )
    FetchContent_MakeAvailable(cli11)
endif()

if(WEBCFACE_FIND_UTF8CPP)
    find_path(UTF8CPP_INCLUDE_DIR utf8.h PATH_SUFFIXES utf8cpp)
endif()
if(WEBCFACE_FIND_UTF8CPP AND NOT UTF8CPP_INCLUDE_DIR STREQUAL "UTF8CPP_INCLUDE_DIR-NOTFOUND")
    set(UTF8CPP_FOUND true)
    message(STATUS "utf8cpp Found: ${UTF8CPP_INCLUDE_DIR}")
else()
    set(UTF8CPP_FOUND false)
    message(STATUS "utf8cpp Not Found, Fetching Source...")
    FetchContent_Declare(utfcpp
        GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
        GIT_TAG v4.0.5
    )
    if(NOT utfcpp_POPULATED)
        FetchContent_Populate(utfcpp)
        set(UTF8CPP_INCLUDE_DIR ${utfcpp_SOURCE_DIR}/source)
    endif()
endif()
add_library(utfcpp INTERFACE)
target_include_directories(utfcpp INTERFACE $<BUILD_INTERFACE:${UTF8CPP_INCLUDE_DIR}>)

if(WEBCFACE_FIND_OPENCV)
    find_package(OpenCV QUIET)
endif()
if(OpenCV_FOUND)
    message(STATUS "OpenCV ${OpenCV_VERSION} Found: ${OpenCV_PREFIX}")
else()
    message(STATUS "OpenCV Not Found")
endif()

# あとでbuildしたmagickppをprefixに追加して再度pkg_check_modulesしているため、
# ここでは毎回cacheをクリアする必要ある
unset(Magickpp_FOUND CACHE)
if(WEBCFACE_FIND_MAGICK)
    pkg_check_modules(Magickpp QUIET Magick++)
endif()
if(Magickpp_FOUND)
    set(Magickpp_INSTALLED 1)
    message(STATUS "Magick++ ${Magickpp_VERSION} Found: ${Magickpp_PREFIX}")
    add_library(magickpp INTERFACE)
    target_include_directories(magickpp INTERFACE ${Magickpp_INCLUDE_DIRS})
    target_compile_options(magickpp INTERFACE ${Magickpp_CFLAGS_OTHER})
    target_link_libraries(magickpp INTERFACE ${Magickpp_LIBRARIES})
    target_link_directories(magickpp INTERFACE ${Magickpp_LIBRARY_DIRS})
else()
    set(Magickpp_INSTALLED 0)
    message(STATUS "Magick++ Not Found")
    include(ProcessorCount)
    ProcessorCount(N)
    if(N EQUAL 0)
        set(N 1)
    endif()

    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        find_path(MAGICKPP_WIN_INCLUDE_DIR Magick++.h
            PATH_SUFFIXES include
        )
        if(NOT MAGICKPP_WIN_INCLUDE_DIR STREQUAL "MAGICKPP_WIN_INCLUDE_DIR-NOTFOUND")
            set(Magickpp_INSTALLED 1)
            get_filename_component(MAGICKPP_PREFIX ${MAGICKPP_WIN_INCLUDE_DIR} DIRECTORY)
            message(STATUS "Magick++ Found: ${MAGICKPP_PREFIX}")
            set(MAGICKPP_LIB_DIR "${MAGICKPP_PREFIX}/lib")
            file(GLOB MAGICKPP_RL_LIBS
                RELATIVE ${MAGICKPP_LIB_DIR}
                ${MAGICKPP_LIB_DIR}/CORE_RL_*.lib
            )
            foreach(lib IN LISTS MAGICKPP_RL_LIBS)
                list(APPEND MAGICKPP_LIBS optimized ${MAGICKPP_LIB_DIR}/${lib})
            endforeach()
            add_library(magickpp INTERFACE)
            target_include_directories(magickpp INTERFACE ${MAGICKPP_WIN_INCLUDE_DIR})
            target_link_libraries(magickpp INTERFACE ${MAGICKPP_LIBS})
            add_compile_definitions(WEBCFACE_MAGICK_VER7)

        else()
            find_program(DEVENV_COMMAND devenv)
            if(DEVENV_COMMAND STREQUAL "DEVENV_COMMAND-NOTFOUND")
                message(FATAL_ERROR "devenv.exe not found")
            endif()
            find_program(MSBUILD_COMMAND msbuild)
            if(MSBUILD_COMMAND STREQUAL "MSBUILD_COMMAND-NOTFOUND")
                message(FATAL_ERROR "msbuild.exe not found")
            endif()
            message(STATUS "Fetching ImageMagick-Windows source...")
            FetchContent_Declare(magick-windows
                GIT_REPOSITORY https://github.com/ImageMagick/ImageMagick-Windows.git
                GIT_TAG 35dbf227258caec73fbe4a0a10ea10f2c2c1051d
            )
            if(NOT EXISTS ${magick-windows_SOURCE_DIR}/IM7.StaticDLL.sln OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${magick-windows_SOURCE_DIR}/IM7.StaticDLL.sln)
                if(NOT EXISTS ${magick-windows_SOURCE_DIR}/Configure/Configure.2017.sln OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${magick-windows_SOURCE_DIR}/Configure/Configure.2017.sln)
                    message(STATUS "Fetching dependencies...")
                    FetchContent_Populate(magick-windows)
                    execute_process(
                        COMMAND CloneRepositories.IM7.cmd
                        WORKING_DIRECTORY ${magick-windows_SOURCE_DIR}
                    )
                    message(STATUS "Building Configure...")
                    execute_process(
                        COMMAND ${DEVENV_COMMAND} /upgrade Configure.2017.sln
                        WORKING_DIRECTORY ${magick-windows_SOURCE_DIR}/Configure
                    )
                    execute_process(
                        COMMAND ${MSBUILD_COMMAND} Configure.2017.sln /m
                        WORKING_DIRECTORY ${magick-windows_SOURCE_DIR}/Configure
                    )
                endif()
                message(STATUS "Executing Configure...")
                execute_process(
                    COMMAND Configure.exe /noWizard /noAliases /noDpc /noHdri /Q8 /${CMAKE_C_COMPILER_ARCHITECTURE_ID} /VS2017 /smtd
                    WORKING_DIRECTORY ${magick-windows_SOURCE_DIR}/Configure
                )
                execute_process(
                    COMMAND ${DEVENV_COMMAND} /upgrade IM7.StaticDLL.${CMAKE_C_COMPILER_ARCHITECTURE_ID}.sln
                    WORKING_DIRECTORY ${magick-windows_SOURCE_DIR}
                )
            endif()
            message(STATUS "Building ImageMagick...")
            set(MAGICKPP_LIB_DIR "${magick-windows_SOURCE_DIR}/Output/lib")
            if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
                execute_process(
                    COMMAND ${MSBUILD_COMMAND} IM7.StaticDLL.${CMAKE_C_COMPILER_ARCHITECTURE_ID}.sln
                        /m /p:Configuration=Release,Platform=${CMAKE_C_COMPILER_ARCHITECTURE_ID}
                    WORKING_DIRECTORY ${magick-windows_SOURCE_DIR}
                )
                file(GLOB MAGICKPP_RL_LIBS
                    RELATIVE ${MAGICKPP_LIB_DIR}
                    ${MAGICKPP_LIB_DIR}/CORE_RL_*.lib
                )
                foreach(lib IN LISTS MAGICKPP_RL_LIBS)
                    list(APPEND MAGICKPP_LIBS optimized ${MAGICKPP_LIB_DIR}/${lib})
                endforeach()
            endif()
            if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
                execute_process(
                    COMMAND ${MSBUILD_COMMAND} IM7.StaticDLL.${CMAKE_C_COMPILER_ARCHITECTURE_ID}.sln
                        /m /p:Configuration=Debug,Platform=${CMAKE_C_COMPILER_ARCHITECTURE_ID}
                    WORKING_DIRECTORY ${magick-windows_SOURCE_DIR}
                )
                file(GLOB MAGICKPP_DB_LIBS
                    RELATIVE ${MAGICKPP_LIB_DIR}
                    ${MAGICKPP_LIB_DIR}/CORE_DB_*.lib
                )
                foreach(lib IN LISTS MAGICKPP_DB_LIBS)
                    list(APPEND MAGICKPP_LIBS debug ${MAGICKPP_LIB_DIR}/${lib})
                endforeach()
            endif()
            
            add_library(magickpp INTERFACE)
            target_include_directories(magickpp INTERFACE
                $<BUILD_INTERFACE:${magick-windows_SOURCE_DIR}/ImageMagick/Magick++/lib>
                $<BUILD_INTERFACE:${magick-windows_SOURCE_DIR}/ImageMagick>
            )
            target_link_libraries(magickpp INTERFACE ${MAGICKPP_LIBS})
            target_compile_definitions(magickpp INTERFACE STATIC_MAGICK)
            add_compile_definitions(WEBCFACE_MAGICK_VER7)
        endif()
    else()
        set(MAGICKPP_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/prefix)
        find_program(SH_COMMAND sh)
        find_program(ENV_COMMAND env)
        find_program(MAKE_COMMAND make)
        find_program(CHMOD_COMMAND chmod)
        find_program(CYGPATH_COMMAND cygpath)
        if(MINGW AND NOT CYGPATH_COMMAND STREQUAL "CYGPATH_COMMAND-NOTFOUND")
            execute_process(
                COMMAND ${CYGPATH_COMMAND} -u "${MAGICKPP_PREFIX}"
                OUTPUT_VARIABLE MAGICKPP_PREFIX_UNIX
            )
        else()
            set(MAGICKPP_PREFIX_UNIX "${MAGICKPP_PREFIX}")
        endif()
        set(MAGICKPP_FLAGS "-O3")
        if(WEBCFACE_PIC)
            set(MAGICKPP_FLAGS "${MAGICKPP_FLAGS} -fPIC")
        endif()
        if(NOT EXISTS ${magickpp_SOURCE_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${magickpp_SOURCE_DIR}/Makefile)
            file(REMOVE_RECURSE ${MAGICKPP_PREFIX})
        endif()
        unset(libjpeg_FOUND CACHE)
        if(WEBCFACE_FIND_JPEG)
            pkg_check_modules(libjpeg QUIET libjpeg)
        endif()
        if(libjpeg_FOUND)
            message(STATUS "libjpeg ${libjpeg_VERSION} Found: ${libjpeg_PREFIX}")
        else()
            message(STATUS "libjpeg Not Found, Fetching source...")
            FetchContent_Declare(libjpeg-turbo
                GIT_REPOSITORY https://github.com/libjpeg-turbo/libjpeg-turbo.git
                GIT_TAG 3.0.3
            )
            FetchContent_Populate(libjpeg-turbo)
            message(STATUS "Building libjpeg-turbo...")
            execute_process(
                COMMAND ${CMAKE_COMMAND} ${libjpeg-turbo_SOURCE_DIR} -B${libjpeg-turbo_BINARY_DIR}
                    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${MAGICKPP_PREFIX}
                    -DENABLE_SHARED=OFF -DWITH_TURBOJPEG=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=${WEBCFACE_PIC}
            )
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${libjpeg-turbo_BINARY_DIR} -t install -j${N})
        endif()

        unset(libpng_FOUND CACHE)
        if(WEBCFACE_FIND_PNG)
            pkg_check_modules(libpng QUIET libpng)
        endif()
        if(libpng_FOUND)
            message(STATUS "libpng ${libpng_VERSION} Found: ${libpng_PREFIX}")
        else()
            message(STATUS "libpng Not Found")
            unset(zlib_FOUND CACHE)
            if(WEBCFACE_FIND_ZLIB)
                pkg_check_modules(zlib QUIET zlib)
            endif()
            if(zlib_FOUND)
                message(STATUS "zlib ${zlib_VERSION} Found: ${zlib_PREFIX}")
            else()
                message(STATUS "zlib Not Found, Fetching source...")
                FetchContent_Declare(zlib
                    URL https://www.zlib.net/current/zlib.tar.gz
                )
                FetchContent_Populate(zlib)
                message(STATUS "Building zlib...")
                if(NOT EXISTS ${zlib_SOURCE_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${zlib_SOURCE_DIR}/Makefile)
                    execute_process(
                        COMMAND ${ENV_COMMAND}
                            "CC=${ORIGINAL_ENV_CC}" "CFLAGS=${MAGICKPP_FLAGS}"
                            "MAKE=${MAKE_COMMAND}"
                            ${SH_COMMAND} configure
                            --static --prefix=${MAGICKPP_PREFIX}
                        WORKING_DIRECTORY ${zlib_SOURCE_DIR}
                    )
                endif()
                execute_process(
                    COMMAND ${MAKE_COMMAND} -j${N} install
                    WORKING_DIRECTORY ${zlib_SOURCE_DIR}
                )
            endif()
            message(STATUS "Fetching libpng source...")
            FetchContent_Declare(libpng
                URL https://download.sourceforge.net/libpng/libpng-1.6.43.tar.xz
            )
            FetchContent_Populate(libpng)
            message(STATUS "Building libpng...")
            execute_process(
                COMMAND ${CMAKE_COMMAND} ${libpng_SOURCE_DIR} -B${libpng_BINARY_DIR}
                    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${MAGICKPP_PREFIX}
                    -DPNG_SHARED=OFF -DPNG_FRAMEWORK=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF
                    -DCMAKE_POSITION_INDEPENDENT_CODE=${WEBCFACE_PIC}
            )
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${libpng_BINARY_DIR} -t install -j${N})
        endif()

        unset(libwebp_FOUND CACHE)
        if(WEBCFACE_FIND_WEBP)
            pkg_check_modules(libwebp QUIET libwebp)
        endif()
        if(libwebp_FOUND)
            message(STATUS "libwebp ${libwebp_VERSION} Found: ${libwebp_PREFIX}")
        else()
            message(STATUS "libwebp Not Found, Fetching source...")
            FetchContent_Declare(libwebp
                GIT_REPOSITORY https://chromium.googlesource.com/webm/libwebp
                GIT_TAG v1.4.0
            )
            FetchContent_Populate(libwebp)
            message(STATUS "Building libwebp...")
            execute_process(
                COMMAND ${CMAKE_COMMAND} ${libwebp_SOURCE_DIR} -B${libwebp_BINARY_DIR}
                    -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${MAGICKPP_PREFIX}
                    -DBUILD_SHARED_LIBS=OFF -DWEBP_LINK_STATIC=ON
                    -DWEBP_BUILD_ANIM_UTILS=OFF -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF
                    -DWEBP_BUILD_GIF2WEBP=OFF -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF
                    -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF
                    -DCMAKE_POSITION_INDEPENDENT_CODE=${WEBCFACE_PIC}
            )
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${libwebp_BINARY_DIR} -t install -j${N})
        endif()

        list(APPEND CMAKE_PREFIX_PATH ${MAGICKPP_PREFIX})
        if(MINGW)
            set(ENV{PKG_CONFIG_PATH} "${MAGICKPP_PREFIX}/lib/pkgconfig;$ENV{PKG_CONFIG_PATH}")
        else()
            set(ENV{PKG_CONFIG_PATH} "${MAGICKPP_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
        endif()
        pkg_check_modules(libjpeg REQUIRED QUIET libjpeg)
        pkg_check_modules(libpng REQUIRED QUIET libpng)
        pkg_check_modules(zlib REQUIRED QUIET zlib)
        pkg_check_modules(libwebp REQUIRED QUIET libwebp)

        message(STATUS "Fetching Magick++ source...")
        FetchContent_Declare(magickpp
            GIT_REPOSITORY https://github.com/ImageMagick/ImageMagick.git
            GIT_TAG 7.1.1-33
        )
        FetchContent_Populate(magickpp)
        message(STATUS "Building Magick++...")
        if(NOT EXISTS ${magickpp_SOURCE_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${magickpp_SOURCE_DIR}/Makefile)
            if(MINGW)
                execute_process(
                    COMMAND ${CHMOD_COMMAND} +x winpath.sh # バグ?
                    WORKING_DIRECTORY ${magickpp_SOURCE_DIR}
                )
            endif()
            execute_process(
                # mingwでは ./configure はつかえない
                COMMAND ${SH_COMMAND} configure
                    "CC=${ORIGINAL_ENV_CC}" "CXX=${ORIGINAL_ENV_CXX}"
                    "CFLAGS=${MAGICKPP_FLAGS}" "CXXFLAGS=${MAGICKPP_FLAGS}"
                    "MAKE=${MAKE_COMMAND}"
                    --prefix=${MAGICKPP_PREFIX_UNIX} --disable-shared --enable-static
                    --without-utilities --disable-hdri --with-quantum-depth=8
                    --without-modules --without-perl --without-bzlib --without-djvu --without-dps
                    --without-fontconfig --without-freetype --without-gvc --without-heic
                    --without-jbig --without-jxl --without-dmr --without-lqr --without-lcms
                    --without-lzma --without-openexr --without-openjp2 --without-pango
                    --without-raqm --without-raw --without-tiff --without-wmf
                    --without-xml --without-zlib --without-zstd --without-x --without-zip
                    --disable-gomp
                WORKING_DIRECTORY ${magickpp_SOURCE_DIR}
            )
        endif()
        execute_process(
            COMMAND ${MAKE_COMMAND} -j${N} install
            WORKING_DIRECTORY ${magickpp_SOURCE_DIR}
        )
        pkg_check_modules(Magickpp REQUIRED QUIET Magick++)
        add_library(magickpp INTERFACE)
        target_include_directories(magickpp INTERFACE $<BUILD_INTERFACE:${Magickpp_STATIC_INCLUDE_DIRS}>)
        target_compile_options(magickpp INTERFACE ${Magickpp_STATIC_CFLAGS_OTHER})
        # 先に書いたものが後に書いたものに依存するらしい
        set(Magickpp_STATIC_LIBRARIES
            ${Magickpp_STATIC_LIBRARIES}
            ${libwebp_STATIC_LIBRARIES}
            ${libpng_STATIC_LIBRARIES}
            ${zlib_STATIC_LIBRARIES}
            ${libjpeg_STATIC_LIBRARIES}
        )
        list(APPEND Magickpp_STATIC_LIBRARY_DIRS
            ${libjpeg_STATIC_LIBRARY_DIRS}
            ${libpng_STATIC_LIBRARY_DIRS}
            ${zlib_STATIC_LIBRARY_DIRS}
            ${libwebp_STATIC_LIBRARY_DIRS}
        )
        list(REMOVE_DUPLICATES Magickpp_STATIC_LIBRARY_DIRS)
        list(FILTER Magickpp_STATIC_LIBRARY_DIRS EXCLUDE REGEX "${MAGICKPP_PREFIX}/lib")
        target_link_libraries(magickpp INTERFACE ${Magickpp_STATIC_LIBRARIES})
        target_link_directories(magickpp INTERFACE
            $<BUILD_INTERFACE:${MAGICKPP_PREFIX}/lib>
            $<INSTALL_INTERFACE:lib>
            ${Magickpp_STATIC_LIBRARY_DIRS}
        )
        if(WIN32)
            target_link_libraries(magickpp INTERFACE urlmon.lib)
        endif()
    endif()
endif()
if(Magickpp_VERSION MATCHES "^7\.")
    add_compile_definitions(WEBCFACE_MAGICK_VER7)
endif()

if(WEBCFACE_TEST)
    message(STATUS "Fetching GTest Source...")
    FetchContent_Declare(googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
    )
    set(INSTALL_GTEST off CACHE INTERNAL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

###############################################################################
include(CheckCXXCompilerFlag)
function(add_cxx_compile_options_if FLAG NAME)
    check_cxx_compiler_flag(${FLAG} FLAG_${NAME}_EXISTS)
    if(FLAG_${NAME}_EXISTS)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:${FLAG}>)
    endif()
endfunction()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options(-Wno-error=deprecated-declarations)
    add_compile_options(-Wno-error=psabi) # refer to PR#9
    # crow
    add_compile_options(-Wno-error=type-limits)
    # inside opencv
    add_cxx_compile_options_if(-Wno-error=deprecated-enum-enum-conversion DEPRECATED_ENUM)
    # inside fmt (fmtlib/fmt#3354)
    add_compile_options(-Wno-error=array-bounds -Wno-error=stringop-overflow)
    # (fmtlib/fmt#3415)
    add_cxx_compile_options_if(-Wno-error=dangling-reference DANGLING_REFERENCE)
    # inside magick
    add_compile_options(-Wno-error=ignored-qualifiers)
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options(-Wno-error=deprecated-declarations)
    add_compile_options(
        --system-header-prefix=msgpack.hpp
        --system-header-prefix=eventpp/
        --system-header-prefix=spdlog/
        --system-header-prefix=curl/
        --system-header-prefix=crow.h
        --system-header-prefix=Magick++.h
        --system-header-prefix=opencv2/
        --system-header-prefix=utf8.h
        --system-header-prefix=CLI/
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/utf-8 /W4 /bigobj)
endif()

set(CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
list(APPEND SVR_MAIN_SRC ${CPP_DIR}/server_main.cc)
list(APPEND WEBCFACE_SRC
    ${CPP_DIR}/message/message.cc
    ${CPP_DIR}/message/unix_path.cc
    ${CPP_DIR}/server/member_data.cc
    ${CPP_DIR}/server/store.cc
    ${CPP_DIR}/server/websock.cc
    ${CPP_DIR}/server/dir.cc
    ${CPP_DIR}/server/ip.cc
    ${CPP_DIR}/client/client.cc
    ${CPP_DIR}/client/client_ws.cc
    ${CPP_DIR}/client/data_store1.cc
    ${CPP_DIR}/client/data_store2.cc
    ${CPP_DIR}/client/encoding.cc
    ${CPP_DIR}/client/field.cc
    ${CPP_DIR}/client/func.cc
    ${CPP_DIR}/client/func_listener.cc
    ${CPP_DIR}/client/log.cc
    ${CPP_DIR}/client/logger.cc
    ${CPP_DIR}/client/member.cc
    ${CPP_DIR}/client/text.cc
    ${CPP_DIR}/client/robot_model.cc
    ${CPP_DIR}/client/canvas3d.cc
    ${CPP_DIR}/client/canvas2d.cc
    ${CPP_DIR}/client/canvas_data.cc
    ${CPP_DIR}/client/value.cc
    ${CPP_DIR}/client/view.cc
    ${CPP_DIR}/client/image.cc
    ${CPP_DIR}/client/version.cc
    ${CPP_DIR}/c_wcf/client.cc
    ${CPP_DIR}/c_wcf/value.cc
    ${CPP_DIR}/c_wcf/text.cc
    ${CPP_DIR}/c_wcf/func.cc
    ${CPP_DIR}/c_wcf/view.cc
)
list(APPEND EXAMPLE_SRC
    ${CPP_DIR}/example/value.cc
    ${CPP_DIR}/example/func.cc
    ${CPP_DIR}/example/log.cc
    ${CPP_DIR}/example/view.cc
    ${CPP_DIR}/example/canvas2d.cc
    ${CPP_DIR}/example/canvas3d.cc
    ${CPP_DIR}/example/recv.cc
    ${CPP_DIR}/example/c_example.c
    ${CPP_DIR}/example/benchmark.cc
    ${CPP_DIR}/example/image-send.cc
    ${CPP_DIR}/example/image-recv.cc
)
if(OpenCV_FOUND)
    list(APPEND EXAMPLE_SRC
        ${CPP_DIR}/example/cv-send.cc
        ${CPP_DIR}/example/cv-recv.cc
    )
endif()
list(APPEND TEST_SRC
    ${CPP_DIR}/test/client_data_test.cc
    ${CPP_DIR}/test/data_test.cc
    ${CPP_DIR}/test/func_test.cc
    ${CPP_DIR}/test/func_listener_test.cc
    ${CPP_DIR}/test/view_test.cc
    ${CPP_DIR}/test/robot_model_test.cc
    ${CPP_DIR}/test/canvas3d_test.cc
    ${CPP_DIR}/test/canvas2d_test.cc
    ${CPP_DIR}/test/image_test.cc
    ${CPP_DIR}/test/member_test.cc
    ${CPP_DIR}/test/logger_test.cc
    ${CPP_DIR}/test/client_test.cc
    ${CPP_DIR}/test/dummy_server.cc
    ${CPP_DIR}/test/dummy_client.cc
    ${CPP_DIR}/test/server_test.cc
    ${CPP_DIR}/test/encoding_test.cc
    ${CPP_DIR}/test/c_wcf_test.cc
)
if(WIN32)
    set(VERSION_RC ${CMAKE_CURRENT_BINARY_DIR}/${WEBCFACE_VERSION_RC_PATH})
else()
    set(VERSION_RC "")
endif()

set(INCLUDE_DIR
    $<BUILD_INTERFACE:${CPP_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

###############################################################################

list(APPEND WEBCFACE_PRIVATE_DEPS
    ${libcurl}
    msgpack-cxx
    ${Crow}
    utfcpp
    magickpp
)
list(APPEND WEBCFACE_PUBLIC_DEPS
    eventpp::eventpp
    ${spdlog}
)
if(WIN32)
    list(APPEND WEBCFACE_PRIVATE_DEPS
        ws2_32.lib wsock32.lib iphlpapi.lib
    )
endif()

if(WEBCFACE_SHARED)
    add_library(${PROJECT_NAME} SHARED ${WEBCFACE_SRC} ${VERSION_RC})
else()
    add_library(${PROJECT_NAME} STATIC ${WEBCFACE_SRC})
    get_target_property(MAGICKPP_TARGET_LINK_DIR magickpp INTERFACE_LINK_DIRECTORIES)
    target_link_directories(${PROJECT_NAME} PUBLIC ${MAGICKPP_TARGET_LINK_DIR})
endif()
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE ${WEBCFACE_PRIVATE_DEPS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${WEBCFACE_PUBLIC_DEPS})
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        DEBUG_POSTFIX "${WEBCFACE_SOVERSION}d"
        RELEASE_POSTFIX "${WEBCFACE_SOVERSION}"
    )
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${WEBCFACE_SOVERSION}
)
if(NOT "${WEBCFACE_CLANG_TIDY}" STREQUAL "")
    set(CLANG_TIDY_CMD "${WEBCFACE_CLANG_TIDY}" "--header-filter=${CMAKE_CURRENT_SOURCE_DIR}/src")
    set_target_properties(${PROJECT_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
endif()
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_library(wcf INTERFACE)
target_include_directories(wcf INTERFACE ${INCLUDE_DIR})
target_compile_features(wcf INTERFACE c_std_99)
if(WEBCFACE_SHARED)
    # sharedの場合はlibc++とリンクする必要もない
    target_link_libraries(wcf INTERFACE $<TARGET_LINKER_FILE:${PROJECT_NAME}::${PROJECT_NAME}>)
else()
    target_link_libraries(wcf INTERFACE ${PROJECT_NAME})
endif()
add_library(${PROJECT_NAME}::wcf ALIAS wcf)

add_executable(${PROJECT_NAME}-server ${SVR_MAIN_SRC} ${VERSION_RC})
target_link_libraries(${PROJECT_NAME}-server PRIVATE
    ${PROJECT_NAME}
    CLI11::CLI11
    msgpack-cxx
)
set_target_properties(${PROJECT_NAME}-server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
target_compile_definitions(${PROJECT_NAME}-server PRIVATE
    FILE_DESCRIPTION="WebCFace Server"
    FILE_IS_APP
)

if(WEBCFACE_EXAMPLE)
    foreach(SRC IN LISTS EXAMPLE_SRC)
        get_filename_component(TARGET ${SRC} NAME_WE)
        add_executable(${PROJECT_NAME}-example-${TARGET} ${SRC} ${VERSION_RC})
        target_link_libraries(${PROJECT_NAME}-example-${TARGET} PRIVATE ${PROJECT_NAME})
        set(WEBCFACE_EXAMPLE_TARGETS
            ${WEBCFACE_EXAMPLE_TARGETS}
            ${PROJECT_NAME}-example-${TARGET}
        )
        set(WEBCFACE_EXAMPLE_TARGETS_CPACK
            ${WEBCFACE_EXAMPLE_TARGETS_CPACK}
            ${PROJECT_NAME}-example-${TARGET} "Example ${TARGET}"
        )
        set_target_properties(${PROJECT_NAME}-example-${TARGET} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
        target_compile_definitions(${PROJECT_NAME}-example-${TARGET} PRIVATE
            FILE_DESCRIPTION="WebCFace Example ${TARGET}"
            FILE_IS_APP
        )
    endforeach()
    foreach(TARGET image-recv image-send)
        target_link_libraries(${PROJECT_NAME}-example-${TARGET} PRIVATE
            magickpp
        )
    endforeach()
    if(OpenCV_FOUND)
        foreach(TARGET cv-recv cv-send)
            target_link_libraries(${PROJECT_NAME}-example-${TARGET} PRIVATE
                opencv_core
                opencv_imgcodecs
                opencv_imgproc
                opencv_highgui
            )
        endforeach()
    endif()
endif()

if(WEBCFACE_TEST)
    enable_testing()
    add_executable(${PROJECT_NAME}-test ${TEST_SRC})
    target_link_libraries(${PROJECT_NAME}-test PRIVATE
        ${PROJECT_NAME}
        GTest::gtest_main
        ${WEBCFACE_PRIVATE_DEPS}
    )
    target_compile_definitions(${PROJECT_NAME}-test PRIVATE
        WEBCFACE_TEST_TIMEOUT=${WEBCFACE_TEST_TIMEOUT}
    )
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}-test DISCOVERY_TIMEOUT 600)
endif()

###############################################################################
if(WEBCFACE_INSTALL)
    include(CMakePackageConfigHelpers)

    # binaries
    install(TARGETS ${PROJECT_NAME}-server
        RUNTIME DESTINATION bin
    )
    install(TARGETS ${WEBCFACE_EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
    )

    # library
    list(APPEND WEBCFACE_EXPORTS ${PROJECT_NAME} wcf)
    list(APPEND WEBCFACE_PKGCONFIG_LIBS -lwebcface)
    # public dependencies
    if(spdlog_FOUND)
        list(APPEND WEBCFACE_PKGCONFIG_REQUIRES spdlog)
        set(SPDLOG_INSTALLED 1)
    else()
        list(APPEND WEBCFACE_EXPORTS ${spdlog})
        set(SPDLOG_INSTALLED 0)
    endif()
    if(eventpp_FOUND)
        set(EVENTPP_INSTALLED 1)
    else()
        list(APPEND WEBCFACE_EXPORTS eventpp)
        set(EVENTPP_INSTALLED 0)
    endif()
    if(NOT WEBCFACE_SHARED)
        # private static dependencies
        if(NOT msgpack-cxx_FOUND)
            list(APPEND WEBCFACE_EXPORTS msgpack-cxx)
        endif()
        if(NOT CURL_FOUND)
            list(APPEND WEBCFACE_EXPORTS ${libcurl})
            if(NOT WEBCFACE_SHARED)
                if(CMAKE_BUILD_TYPE STREQUAL Debug)
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -lcurl-d)
                else()
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -lcurl)
                endif()
            endif()
        endif()
        if(NOT Crow_FOUND)
            list(APPEND WEBCFACE_EXPORTS Crow)
        endif()
        if(NOT UTF8CPP_FOUND)
            list(APPEND WEBCFACE_EXPORTS utfcpp)
        endif()
        if(Magickpp_INSTALLED)
            if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                list(APPEND WEBCFACE_PKGCONFIG_LIBS -L${MAGICKPP_LIB_DIR})
                foreach(lib IN LISTS MAGICKPP_DB_LIBS MAGICKPP_RL_LIBS)
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -l${lib})
                endforeach()
            else()
                list(APPEND WEBCFACE_PKGCONFIG_REQUIRES Magickpp)
            endif()
        else()
            list(APPEND WEBCFACE_EXPORTS magickpp)
            if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
                foreach(lib IN LISTS MAGICKPP_DB_LIBS MAGICKPP_RL_LIBS)
                    install(FILES ${MAGICKPP_LIB_DIR}/${lib}
                        DESTINATION lib
                    )
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -l${lib})
                endforeach()
            else()
                foreach(dir IN LISTS Magickpp_STATIC_LIBRARY_DIRS)
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -L${dir})
                endforeach()
                foreach(lib IN LISTS Magickpp_STATIC_LIBRARIES)
                    install(FILES ${MAGICKPP_PREFIX}/lib/lib${lib}.a
                        DESTINATION lib
                        OPTIONAL
                    )
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -l${lib})
                endforeach()
            endif()
        endif()
    endif()
    install(TARGETS ${WEBCFACE_EXPORTS}
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if(WEBCFACE_SHARED)
            get_target_property(WEBCFACE_PDB ${PROJECT_NAME} PDB_NAME)
        else()
            get_target_property(WEBCFACE_PDB ${PROJECT_NAME} COMPILE_PDB_NAME)
        endif()
        install(FILES ${WEBCFACE_PDB}
            DESTINATION bin
            OPTIONAL
        )
    endif()
    install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
    )
    set(PACKAGE_INIT "@PACKAGE_INIT@")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in
        @ONLY
    )
    unset(PACKAGE_INIT)
    configure_package_config_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake.in
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

    # pkg-config
    list(JOIN WEBCFACE_PKGCONFIG_LIBS " " WEBCFACE_PKGCONFIG_LIBS)
    list(JOIN WEBCFACE_PKGCONFIG_REQUIRES " " WEBCFACE_PKGCONFIG_REQUIRES)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION lib/pkgconfig
    )

    # include
    install(DIRECTORY ${CPP_DIR}/include/
        DESTINATION include
    )
    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION include
    )
    if(NOT spdlog_FOUND)
        install(DIRECTORY ${spdlog_SOURCE_DIR}/include/
            DESTINATION include
        )
    endif()
    if(NOT eventpp_FOUND)
        install(DIRECTORY ${eventpp_SOURCE_DIR}/include/
            DESTINATION include
        )
    endif()

    # service
    if(WEBCFACE_INSTALL_SERVICE)
        install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/webcface-server.service
            DESTINATION lib/systemd/system
        )
    endif()

    # webui
    if(WEBCFACE_DOWNLOAD_WEBUI)
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/
            DESTINATION share/webcface/dist
        )
    endif()

    # licenses
    install(FILES
        ${CMAKE_CURRENT_LIST_DIR}/README.md
        ${CMAKE_CURRENT_LIST_DIR}/LICENSE
        DESTINATION share/webcface
    )
    if(NOT msgpack-cxx_FOUND)
        install(FILES
            ${msgpack-cxx_SOURCE_DIR}/LICENSE_1_0.txt
            DESTINATION share/webcface/3rd_party/msgpack-c
        )
    endif()
    if(NOT eventpp_FOUND)
        install(FILES
            ${eventpp_SOURCE_DIR}/license
            DESTINATION share/webcface/3rd_party/eventpp
        )
    endif()
    if(NOT spdlog_FOUND)
        install(FILES
            ${spdlog_SOURCE_DIR}/LICENSE
            DESTINATION share/webcface/3rd_party/spdlog
        )
    endif()
    # curl: リポジトリ内にないしバイナリ配布に必須ではないのでスキップ
    if(NOT CROW_FOUND)
        if(NOT ASIO_FOUND)
            install(FILES
                ${asio_SOURCE_DIR}/asio/LICENSE_1_0.txt
                DESTINATION share/webcface/3rd_party/asio
            )
        endif()
        install(FILES
            ${crow_SOURCE_DIR}/LICENSE
            ${crow_SOURCE_DIR}/README.md
            # 3rdpartyのライセンスがREADMEにある
            DESTINATION share/webcface/3rd_party/crow
        )
    endif()
    if(NOT CLI11_FOUND)
        install(FILES
            ${cli11_SOURCE_DIR}/LICENSE
            DESTINATION share/webcface/3rd_party/cli11
        )
    endif()
    if(NOT UTF8CPP_FOUND)
        install(FILES
            ${utfcpp_SOURCE_DIR}/LICENSE
            DESTINATION share/webcface/3rd_party/utfcpp
        )
    endif()
    if(NOT Magickpp_INSTALLED)
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            install(FILES
                ${magick-windows_SOURCE_DIR}/ImageMagick/LICENSE
                DESTINATION share/webcface/3rd_party/ImageMagick
            )
        else()
            install(FILES
                ${magickpp_SOURCE_DIR}/LICENSE
                DESTINATION share/webcface/3rd_party/ImageMagick
            )
        endif()
    endif()

    # cpack
    set(CPACK_VERBATIM_VARIABLES yes)
    set(CPACK_PACKAGE_CONTACT ${WEBCFACE_CONTACT})
    set(CPACK_PACKAGE_VENDOR ${WEBCFACE_CONTACT})
    set(CPACK_PACKAGE_EXECUTABLES
        webcface-server "WebCFace Server"
        ${WEBCFACE_EXAMPLE_TARGETS_CPACK}
    )
    set(CPACK_STRIP_FILES yes)
    set(CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
    )
    set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_LIST_DIR}/LICENSE)
    
    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_VERSION ${WEBCFACE_VERSION})
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "na-trium-144 <${CPACK_PACKAGE_CONTACT}>")
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS yes)
    # dev package for public dependencies
    if(${spdlog_FOUND})
        set(CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} libspdlog-dev)
    endif()
    # https://gitlab.kitware.com/cmake/cmake/-/issues/21834
    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_LIST_DIR}/cmake/triggers")
    set(CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION TRUE)

    set(CPACK_NSIS_DISPLAY_NAME "WebCFace ${WEBCFACE_VERSION}")
    set(CPACK_NSIS_MODIFY_PATH on)
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL on)

    include(CPack)

    if(ament_cmake_FOUND)
        ament_export_targets(${PROJECT_NAME}-targets HAS_LIBRARY_TARGET)
        ament_package()
    endif()
endif()

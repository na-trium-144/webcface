cmake_minimum_required(VERSION 3.12)
# ubuntu20.04: cmake 3.16.3

set(ORIGINAL_ENV_CC "$ENV{CC}" CACHE STRING "")
set(ORIGINAL_ENV_CXX "$ENV{CXX}" CACHE STRING "")

project(webcface
    LANGUAGES CXX C
    VERSION 1.11.4
    HOMEPAGE_URL "https://github.com/na-trium-144/webcface"
    DESCRIPTION "Web-based IPC & Dashboard-like UI"
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
option(WEBCFACE_FIND_LIBS "if false, disables every find_package and pkg_check_modules" ${WEBCFACE_SHARED})
option(WEBCFACE_FIND_MSGPACK "try find_package(msgpack-cxx)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_EVENTPP "try find_package(eventpp)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_SPDLOG "try find_package(spdlog)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_CURL "try find_package(CURL)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_ASIO "try find_path(asio.hpp)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_CROW "try find_package(Crow)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_CLI11 "try find_package(CLI11)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_UTF8CPP "try find_path(utf8.h)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_OPENCV "try find_package(OpenCV) (only for example)" OFF)
option(WEBCFACE_FIND_MAGICK "try pkg_check_modules(Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_JPEG "try pkg_check_modules(libjpeg) (while building Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_PNG "try pkg_check_modules(libpng) (while building Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_ZLIB "try find_package(zlib) (while building libpng)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_WEBP "try pkg_check_modules(libwebp) (while building Magick++)" ${WEBCFACE_FIND_LIBS})
option(WEBCFACE_FIND_AMENT "try find_package(ament_cmake)" ON)

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
message(STATUS "webcface_find_eventpp = ${WEBCFACE_FIND_EVENTPP}")
message(STATUS "webcface_find_spdlog = ${WEBCFACE_FIND_SPDLOG}")
message(STATUS "webcface_find_curl = ${WEBCFACE_FIND_CURL}")
message(STATUS "webcface_find_asio = ${WEBCFACE_FIND_ASIO}")
message(STATUS "webcface_find_crow = ${WEBCFACE_FIND_CROW}")
message(STATUS "webcface_find_cli11 = ${WEBCFACE_FIND_CLI11}")
message(STATUS "webcface_find_utf8cpp = ${WEBCFACE_FIND_UTF8CPP}")
message(STATUS "webcface_find_opencv = ${WEBCFACE_FIND_OPENCV}")
message(STATUS "webcface_find_magick = ${WEBCFACE_FIND_MAGICK}")
message(STATUS "webcface_find_jpeg = ${WEBCFACE_FIND_JPEG}")
message(STATUS "webcface_find_png = ${WEBCFACE_FIND_PNG}")
message(STATUS "webcface_find_zlib = ${WEBCFACE_FIND_ZLIB}")
message(STATUS "webcface_find_webp = ${WEBCFACE_FIND_WEBP}")
message(STATUS "webcface_find_ament = ${WEBCFACE_FIND_AMENT}")

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

# FetchContentでは名前はlower caseになるので、それで統一
# (必ず名前もディレクトリ名もlower caseにすること)
macro(fetch_cmake DEP_NAME URL TAG)
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/external/${DEP_NAME}/CMakeLists.txt)
        message(STATUS "Using Source at external/${DEP_NAME}")
        set(${DEP_NAME}_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/external/${DEP_NAME})
        set(${DEP_NAME}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/${DEP_NAME}-build)
        add_subdirectory(external/${DEP_NAME} _deps/${DEP_NAME}-build)
    else()
        message(STATUS "Fetching Source from ${URL}...")
        FetchContent_Declare(${DEP_NAME} GIT_REPOSITORY ${URL} GIT_TAG ${TAG})
        FetchContent_MakeAvailable(${DEP_NAME})
    endif()
endmacro()
macro(fetch_only DEP_NAME URL TAG CHECK_FILENAME)
    if(EXISTS ${CMAKE_CURRENT_LIST_DIR}/external/${DEP_NAME}/${CHECK_FILENAME})
        message(STATUS "Using Source at external/${DEP_NAME}")
        set(${DEP_NAME}_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/external/${DEP_NAME})
        set(${DEP_NAME}_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/_deps/${DEP_NAME}-build)
    else()
        message(STATUS "Fetching Source from ${URL}...")
        FetchContent_Declare(${DEP_NAME} GIT_REPOSITORY ${URL} GIT_TAG ${TAG})
        FetchContent_Populate(${DEP_NAME})
    endif()
endmacro()

if(WEBCFACE_FIND_MSGPACK)
    find_package(msgpack-cxx QUIET)
endif()
if(msgpack-cxx_FOUND)
    message(STATUS "msgpack-cxx ${msgpack-cxx_VERSION} Found: ${msgpack-cxx_DIR}")
else()
    message(STATUS "msgpack-cxx Not Found")
    # msgpackのcmakelistsを使うとmsgpackをインストールしてしまうので、includeするだけ
    fetch_only(msgpack-c
        https://github.com/msgpack/msgpack-c.git
        cpp-6.1.0
        include
    )
    add_library(msgpack-cxx INTERFACE)
    target_compile_definitions(msgpack-cxx INTERFACE
        MSGPACK_NO_BOOST
        MSGPACK_DEFAULT_API_VERSION=3
    )
    target_include_directories(msgpack-cxx INTERFACE $<BUILD_INTERFACE:${msgpack-c_SOURCE_DIR}/include>)
endif()

if(WEBCFACE_FIND_EVENTPP)
    find_package(eventpp QUIET)
endif()
if(eventpp_FOUND)
    message(STATUS "eventpp ${eventpp_VERSION} Found: ${eventpp_DIR}")
else()
    message(STATUS "eventpp Not Found")
    set(EVENTPP_INSTALL OFF CACHE INTERNAL "" FORCE)
    fetch_cmake(eventpp
        https://github.com/wqking/eventpp.git
        v0.1.3
    )
endif()

if(WEBCFACE_FIND_SPDLOG)
    find_package(spdlog QUIET)
endif()
if(spdlog_FOUND)
    message(STATUS "spdlog ${spdlog_VERSION} Found: ${spdlog_DIR}")
    set(spdlog spdlog::spdlog)
else()
    message(STATUS "spdlog Not Found")
    set(SPDLOG_INSTALL OFF CACHE INTERNAL "" FORCE)
    if(WIN32)
        set(SPDLOG_WCHAR_SUPPORT ON CACHE INTERNAL "" FORCE)
    endif()
    fetch_cmake(spdlog
        https://github.com/gabime/spdlog.git
        v1.12.0
    )
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
    message(STATUS "CURL Not Found")
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
    fetch_cmake(curl
        https://github.com/curl/curl.git
        curl-8_5_0
    )
    set(libcurl libcurl_static)
endif()
add_library(libcurl-linker INTERFACE)
if(WEBCFACE_SHARED AND APPLE)
    if(NOT CURL_FOUND)
        target_compile_definitions(libcurl-linker INTERFACE CURL_STATICLIB)
    endif()
    target_link_directories(libcurl-linker INTERFACE $<TARGET_LINKER_FILE_DIR:${libcurl}>)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        # なぜかTARGET_LINKER_FILE_BASE_NAMEすると"libcurl"になってしまう
        target_link_libraries(libcurl-linker INTERFACE -Wl,-hidden-lcurl-d)
    else()
        target_link_libraries(libcurl-linker INTERFACE -Wl,-hidden-lcurl)
    endif()
    target_link_libraries(libcurl-linker INTERFACE $<TARGET_PROPERTY:${libcurl},LINK_LIBRARIES>)
    add_dependencies(libcurl-linker ${libcurl})
else()
    target_link_libraries(libcurl-linker INTERFACE ${libcurl})
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
        message(STATUS "Asio Not Found")
        fetch_only(asio
            https://github.com/chriskohlhoff/asio.git
            asio-1-29-0
            asio
        )
        set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE INTERNAL "" FORCE)
    endif()

    # asioがない場合find_packageしないようにするため、
    # add_subdirectoryではなくinterfaceライブラリを定義する
    fetch_only(crow
        # https://github.com/CrowCpp/Crow.git
        # 921ce6f
        https://github.com/na-trium-144/Crow.git
        5f5372ed80860dfcef788972bb0fd3972f715842
        CMakeLists.txt
    )
    add_library(Crow INTERFACE)
    target_include_directories(Crow INTERFACE
        $<BUILD_INTERFACE:${crow_SOURCE_DIR}/include>
        $<BUILD_INTERFACE:${ASIO_INCLUDE_DIR}>
    )
    set(Crow Crow)
endif()
add_compile_definitions(ASIO_DISABLE_VISIBILITY)

if(WEBCFACE_FIND_CLI11)
    find_package(CLI11 QUIET)
endif()
if(CLI11_FOUND)
    message(STATUS "CLI11 ${CLI11_VERSION} Found: ${CLI11_DIR}")
else()
    message(STATUS "CLI11 Not Found")
    fetch_cmake(cli11
        https://github.com/CLIUtils/CLI11.git
        v2.3.2
    )
endif()

if(WEBCFACE_FIND_UTF8CPP)
    find_path(UTF8CPP_INCLUDE_DIR utf8.h PATH_SUFFIXES utf8cpp)
endif()
if(WEBCFACE_FIND_UTF8CPP AND NOT UTF8CPP_INCLUDE_DIR STREQUAL "UTF8CPP_INCLUDE_DIR-NOTFOUND")
    set(UTF8CPP_FOUND true)
    message(STATUS "utf8cpp Found: ${UTF8CPP_INCLUDE_DIR}")
else()
    set(UTF8CPP_FOUND false)
    message(STATUS "utf8cpp Not Found")
    fetch_only(utfcpp
        https://github.com/nemtrif/utfcpp.git
        v4.0.5
        source
    )
    set(UTF8CPP_INCLUDE_DIR ${utfcpp_SOURCE_DIR}/source)
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
    target_link_directories(magickpp INTERFACE ${Magickpp_LIBRARY_DIRS})
    target_link_libraries(magickpp INTERFACE ${Magickpp_LIBRARIES})
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
        if(WEBCFACE_FIND_MAGICK AND NOT MAGICKPP_WIN_INCLUDE_DIR STREQUAL "MAGICKPP_WIN_INCLUDE_DIR-NOTFOUND")
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
            # submoduleだとCloneRepositories.cmdがうまくいかない
            FetchContent_Declare(imagemagick-windows
                GIT_REPOSITORY https://github.com/ImageMagick/ImageMagick-Windows.git
                GIT_TAG 35dbf227258caec73fbe4a0a10ea10f2c2c1051d
            )
            FetchContent_Populate(imagemagick-windows)
            # https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170
            # https://stackoverflow.com/questions/33380128/visual-studio-2015-command-line-retarget-solution
            if(CMAKE_CXX_COMPILER_VERSION STRGREATER "19.30")
                set(VS_TOOLCHAIN v143)
                set(VS_VERSION 2022)
            elseif(CMAKE_CXX_COMPILER_VERSION STRGREATER "19.20")
                set(VS_TOOLCHAIN v142)
                set(VS_VERSION 2019)
            elseif(CMAKE_CXX_COMPILER_VERSION STRGREATER "19.10")
                set(VS_TOOLCHAIN v141)
                set(VS_VERSION 2017)
            else()
                message(FATAL_ERROR "Compiler version ${CMAKE_CXX_COMPILER_VERSION} is older than vs2017(19.10)")
            endif()
            if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/IM7.StaticDLL.sln OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick-windows_SOURCE_DIR}/IM7.StaticDLL.sln)
                if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/Configure/Configure.exe OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick-windows_SOURCE_DIR}/Configure/Configure.exe)
                    message(STATUS "Fetching dependencies...")
                    execute_process(
                        COMMAND CloneRepositories.IM7.cmd
                        WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
                    )
                    # patch
                    file(APPEND
                        ${imagemagick-windows_SOURCE_DIR}/Projects/MagickCore/magick-baseconfig.h.in
                        "\n#pragma warning(disable: 4201)\n"
                    )
                    message(STATUS "Building Configure...")
                    set(COMMAND ${DEVENV_COMMAND} /upgrade Configure.2017.sln)
                    message(STATUS "${COMMAND}")
                    execute_process(
                        COMMAND ${COMMAND}
                        WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}/Configure
                    )
                    set(COMMAND ${MSBUILD_COMMAND} Configure.2017.sln /m /p:PlatformToolset=${VS_TOOLCHAIN})
                    message(STATUS "${COMMAND}")
                    execute_process(
                        COMMAND ${COMMAND}
                        WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}/Configure
                    )
                endif()
                if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/Configure/Configure.exe)
                    message(FATAL_ERROR "Failed to build Configure.exe for ImageMagick")
                endif()
                message(STATUS "Executing Configure...")
                set(COMMAND Configure.exe /noWizard /noAliases /noDpc /noHdri /Q8 /${CMAKE_C_COMPILER_ARCHITECTURE_ID} /VS${VS_VERSION} /smtd)
                message(STATUS "${COMMAND}")
                execute_process(
                    COMMAND ${COMMAND}
                    WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}/Configure
                )
                set(COMMAND ${DEVENV_COMMAND} /upgrade IM7.StaticDLL.${CMAKE_C_COMPILER_ARCHITECTURE_ID}.sln)
                message(STATUS "${COMMAND}")
                execute_process(
                    COMMAND ${COMMAND}
                    WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
                )
            endif()
            if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/IM7.StaticDLL.sln)
                message(STATUS "Failed to configure ImageMagick")
            endif()
            message(STATUS "Building ImageMagick...")
            set(MAGICKPP_LIB_DIR "${imagemagick-windows_SOURCE_DIR}/Output/lib")
            if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
                set(COMMAND ${MSBUILD_COMMAND} IM7.StaticDLL.${CMAKE_C_COMPILER_ARCHITECTURE_ID}.sln
                    /m /p:PlatformToolset=${VS_TOOLCHAIN},Configuration=Release,Platform=${CMAKE_C_COMPILER_ARCHITECTURE_ID}
                )
                message(STATUS "${COMMAND}")
                execute_process(
                    COMMAND ${COMMAND}
                    WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
                )
                file(GLOB MAGICKPP_RL_LIBS
                    RELATIVE ${MAGICKPP_LIB_DIR}
                    ${MAGICKPP_LIB_DIR}/CORE_RL_*.lib
                )
                if(MAGICKPP_RL_LIBS STREQUAL "")
                    message(FATAL_ERROR "Failed to build ImageMagick")
                endif()
                foreach(lib IN LISTS MAGICKPP_RL_LIBS)
                    list(APPEND MAGICKPP_LIBS optimized ${MAGICKPP_LIB_DIR}/${lib})
                endforeach()
            endif()
            if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
                set(COMMAND ${MSBUILD_COMMAND} IM7.StaticDLL.${CMAKE_C_COMPILER_ARCHITECTURE_ID}.sln
                    /m /p:Configuration=Debug,Platform=${CMAKE_C_COMPILER_ARCHITECTURE_ID}
                )
                message(STATUS "${COMMAND}")
                execute_process(
                    COMMAND ${COMMAND}
                    WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
                )
                file(GLOB MAGICKPP_DB_LIBS
                    RELATIVE ${MAGICKPP_LIB_DIR}
                    ${MAGICKPP_LIB_DIR}/CORE_DB_*.lib
                )
                if(MAGICKPP_DB_LIBS STREQUAL "")
                    message(FATAL_ERROR "Failed to build ImageMagick")
                endif()
                foreach(lib IN LISTS MAGICKPP_DB_LIBS)
                    list(APPEND MAGICKPP_LIBS debug ${MAGICKPP_LIB_DIR}/${lib})
                endforeach()
            endif()
            
            add_library(magickpp INTERFACE)
            target_include_directories(magickpp INTERFACE
                $<BUILD_INTERFACE:${imagemagick-windows_SOURCE_DIR}/ImageMagick/Magick++/lib>
                $<BUILD_INTERFACE:${imagemagick-windows_SOURCE_DIR}/ImageMagick>
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
        if(NOT EXISTS ${imagemagick_SOURCE_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick_SOURCE_DIR}/Makefile)
            file(REMOVE_RECURSE ${MAGICKPP_PREFIX})
        endif()
        unset(libjpeg_FOUND CACHE)
        if(WEBCFACE_FIND_JPEG)
            pkg_check_modules(libjpeg QUIET libjpeg)
        endif()
        if(libjpeg_FOUND)
            message(STATUS "libjpeg ${libjpeg_VERSION} Found: ${libjpeg_PREFIX}")
        else()
            message(STATUS "libjpeg Not Found")
            fetch_only(libjpeg-turbo
                https://github.com/libjpeg-turbo/libjpeg-turbo.git
                3.0.3
                CMakeLists.txt
            )
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
                message(STATUS "zlib Not Found")
                fetch_only(zlib
                    https://github.com/madler/zlib.git
                    v1.3.1
                    configure
                )
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
            fetch_only(libpng
                https://github.com/pnggroup/libpng.git
                v1.6.43
                CMakeLists.txt
            )
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
            message(STATUS "libwebp Not Found")
            fetch_only(libwebp
                https://github.com/webmproject/libwebp.git
                v1.4.0
                CMakeLists.txt
            )
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

        fetch_only(imagemagick
            https://github.com/ImageMagick/ImageMagick.git
            7.1.1-33
            configure
        )
        message(STATUS "Building Magick++...")
        if(NOT EXISTS ${imagemagick_SOURCE_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick_SOURCE_DIR}/Makefile)
            if(MINGW)
                execute_process(
                    COMMAND ${CHMOD_COMMAND} +x winpath.sh # バグ?
                    WORKING_DIRECTORY ${imagemagick_SOURCE_DIR}
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
                WORKING_DIRECTORY ${imagemagick_SOURCE_DIR}
            )
        endif()
        execute_process(
            COMMAND ${MAKE_COMMAND} -j${N} install
            WORKING_DIRECTORY ${imagemagick_SOURCE_DIR}
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
        target_link_directories(magickpp INTERFACE
            $<BUILD_INTERFACE:${MAGICKPP_PREFIX}/lib>
            $<INSTALL_INTERFACE:lib>
            ${Magickpp_STATIC_LIBRARY_DIRS}
        )
        if(WEBCFACE_SHARED AND APPLE)
            foreach(lib IN LISTS Magickpp_STATIC_LIBRARIES)
                target_link_libraries(magickpp INTERFACE -Wl,-hidden-l${lib})
            endforeach()
        else()
            target_link_libraries(magickpp INTERFACE ${Magickpp_STATIC_LIBRARIES})
        endif()
        if(WIN32)
            target_link_libraries(magickpp INTERFACE urlmon.lib)
        endif()
    endif()
endif()
if(Magickpp_VERSION MATCHES "^7\.")
    add_compile_definitions(WEBCFACE_MAGICK_VER7)
endif()

if(WEBCFACE_TEST)
    set(INSTALL_GTEST off CACHE INTERNAL "" FORCE)
    fetch_cmake(googletest
        https://github.com/google/googletest.git
        v1.14.0
    )
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
    add_compile_options(-Wno-error=ignored-attributes)
    add_compile_options(-Wno-error=attributes)
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
    add_compile_options(-Wno-error=ignored-attributes)
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

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

list(APPEND WEBCFACE_SERVER_IN_SRC
    server-internal/src/server_ws.cc
)
list(APPEND WEBCFACE_ENCODING_SRC
    encoding/src/encoding.cc
    encoding/src/val_adaptor.cc
)
list(APPEND WEBCFACE_MESSAGE_SRC
    message/src/message.cc
)
list(APPEND WEBCFACE_SERVER_SRC
    server-store/src/member_data.cc
    server-store/src/store.cc
    server-store/src/websock.cc
    server-store/src/dir.cc
    server-store/src/ip.cc
    server-store/src/unix_path.cc
)
list(APPEND WEBCFACE_CLIENT_SRC
    client/src/client.cc
    client/src/client_ws.cc
    client/src/data_store1.cc
    client/src/data_store2.cc
    client/src/field.cc
    client/src/func_info.cc
    client/src/func_result.cc
    client/src/func.cc
    client/src/func_listener.cc
    client/src/log.cc
    client/src/logger.cc
    client/src/member.cc
    client/src/text.cc
    client/src/robot_link.cc
    client/src/robot_model.cc
    client/src/canvas3d.cc
    client/src/canvas2d.cc
    client/src/value.cc
    client/src/view.cc
    client/src/image.cc
    client/src/image_frame.cc
    client/src/version.cc
    client/src/component_canvas3d.cc
    client/src/component_canvas2d.cc
    client/src/component_view.cc
    client/src/c_wcf/client.cc
    client/src/c_wcf/value.cc
    client/src/c_wcf/text.cc
    client/src/c_wcf/func.cc
    client/src/c_wcf/view.cc
)
list(APPEND SVR_MAIN_SRC
    server/server_main.cc
)
list(APPEND EXAMPLE_SRC
    examples/value.cc
    examples/func.cc
    examples/log.cc
    examples/view.cc
    examples/canvas2d.cc
    examples/canvas3d.cc
    examples/recv.cc
    examples/c_example.c
    examples/benchmark.cc
    examples/image-send.cc
    examples/image-recv.cc
)
if(OpenCV_FOUND)
    list(APPEND EXAMPLE_SRC
        examples/cv-send.cc
        examples/cv-recv.cc
    )
endif()
list(APPEND TEST_SRC
    tests/client_data_test.cc
    tests/data_test.cc
    tests/func_test.cc
    tests/func_listener_test.cc
    tests/view_test.cc
    tests/robot_model_test.cc
    tests/canvas3d_test.cc
    tests/canvas2d_test.cc
    tests/image_test.cc
    tests/member_test.cc
    tests/logger_test.cc
    tests/client_test.cc
    tests/dummy_server.cc
    tests/dummy_client.cc
    tests/server_test.cc
    tests/encoding_test.cc
    tests/c_wcf_test.cc
)
if(WIN32)
    set(VERSION_RC ${CMAKE_CURRENT_BINARY_DIR}/${WEBCFACE_VERSION_RC_PATH})
else()
    set(VERSION_RC "")
endif()

macro(set_target_postfix TARGET)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_target_properties(${TARGET} PROPERTIES
            DEBUG_POSTFIX "${WEBCFACE_SOVERSION}d"
            RELEASE_POSTFIX "${WEBCFACE_SOVERSION}"
        )
    endif()
endmacro()

###############################################################################
# server-internal: private
if(WEBCFACE_SHARED)
    # Crowとasioをwebcfaceのインタフェースから隔離するためにライブラリを分ける
    # (こうしないとLinuxでどうしてもasioのシンボルがwebcfaceに残ってセグフォする)
    add_library(webcface-server-internal STATIC ${WEBCFACE_SERVER_IN_SRC})
    set_target_postfix(webcface-server-internal)
else()
    add_library(webcface-server-internal OBJECT ${WEBCFACE_SERVER_IN_SRC})
endif()
target_include_directories(webcface-server-internal PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/server-internal/include>)
target_link_libraries(webcface-server-internal PRIVATE ${Crow})

if(WEBCFACE_SHARED)
    add_library(webcface-server-internal-linker INTERFACE)
    if(APPLE)
        # MacOSでは--exclude-libsが使えず、その代わり-hidden-lでライブラリを渡す必要がある
        target_link_directories(webcface-server-internal-linker INTERFACE $<TARGET_LINKER_FILE_DIR:webcface-server-internal>)
        target_link_libraries(webcface-server-internal-linker INTERFACE -Wl,-hidden-lwebcface-server-internal)
        add_dependencies(webcface-server-internal-linker webcface-server-internal)
    else()
        target_link_libraries(webcface-server-internal-linker INTERFACE webcface-server-internal)
    endif()
endif()

# encoding: public
add_library(webcface-encoding OBJECT ${WEBCFACE_ENCODING_SRC})
target_include_directories(webcface-encoding PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/encoding/include>)
target_compile_definitions(webcface-encoding PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-encoding PRIVATE utfcpp)

# message: APIはprivateだがシンボルは公開
add_library(webcface-message OBJECT ${WEBCFACE_MESSAGE_SRC})
target_include_directories(webcface-message PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/message/include>)
target_compile_definitions(webcface-message PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-message PUBLIC
    webcface-encoding
    msgpack-cxx
    utfcpp
    ${spdlog}
)

# server-store: public
add_library(webcface-server-store OBJECT ${WEBCFACE_SERVER_SRC})
target_include_directories(webcface-server-store PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/server-store/include>)
target_compile_definitions(webcface-server-store PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-server-store PRIVATE
    webcface-server-internal
    webcface-encoding
    webcface-message
    magickpp
    ${spdlog}
)

# client: public
add_library(webcface-client OBJECT ${WEBCFACE_CLIENT_SRC})
target_include_directories(webcface-client PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/client/include>)
target_compile_definitions(webcface-client PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-client PRIVATE
    webcface-message
    webcface-server-store
    ${libcurl}
)
target_link_libraries(webcface-client PUBLIC
    webcface-encoding
    eventpp::eventpp
    ${spdlog}
)

# client: public
if(WEBCFACE_SHARED)
    add_library(webcface SHARED ${VERSION_RC})
else()
    add_library(webcface STATIC)
endif()
target_compile_features(webcface PUBLIC cxx_std_20)
target_include_directories(webcface PUBLIC
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:webcface-encoding,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:webcface-server-store,INTERFACE_INCLUDE_DIRECTORIES>>
    $<BUILD_INTERFACE:$<TARGET_PROPERTY:webcface-client,INTERFACE_INCLUDE_DIRECTORIES>>
    $<INSTALL_INTERFACE:include>
)
# object libraryをlink_librariesに渡すとprivateとpublicの依存ライブラリがごっちゃになってうまくいかない
target_sources(webcface PRIVATE
    $<TARGET_OBJECTS:webcface-encoding>
    $<TARGET_OBJECTS:webcface-message>
    $<TARGET_OBJECTS:webcface-server-store>
    $<TARGET_OBJECTS:webcface-client>
)
if(WEBCFACE_SHARED)
    target_link_libraries(webcface PRIVATE webcface-server-internal-linker)
else()
    target_sources(webcface PRIVATE $<TARGET_OBJECTS:webcface-server-internal>)
endif()
# private: リンクが必要なもののみ
target_link_libraries(webcface PRIVATE magickpp libcurl-linker)
if(NOT WEBCFACE_SHARED)
    target_link_directories(webcface PUBLIC
        $<TARGET_PROPERTY:magickpp,INTERFACE_LINK_DIRECTORIES>
        $<TARGET_PROPERTY:libcurl-linker,INTERFACE_LINK_DIRECTORIES>
    )
endif()
# public: header only も含む
target_link_libraries(webcface PUBLIC eventpp::eventpp ${spdlog})
if(WIN32)
    target_link_libraries(webcface PRIVATE ws2_32.lib wsock32.lib iphlpapi.lib)
endif()
if(WEBCFACE_SHARED AND NOT WIN32)
    # webcfaceのAPI以外をexportしない。
    # MSVCではデフォルトでdllexportしたもの以外exportされないので問題なし
    set_target_properties(webcface-server-internal PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(webcface-encoding PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(webcface-message PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(webcface-server-store PROPERTIES CXX_VISIBILITY_PRESET hidden)
    set_target_properties(webcface-client PROPERTIES CXX_VISIBILITY_PRESET hidden)
    if(NOT APPLE)
        # object libraryのwebcface-messageなどはexcludeされない
        target_link_options(webcface PRIVATE -Wl,--exclude-libs,ALL)
    endif()
endif()
set_target_postfix(webcface)
set_target_properties(webcface PROPERTIES
    INSTALL_RPATH "$ORIGIN/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${WEBCFACE_SOVERSION}
)
if(NOT "${WEBCFACE_CLANG_TIDY}" STREQUAL "")
    set(CLANG_TIDY_CMD "${WEBCFACE_CLANG_TIDY}" "--header-filter=${CMAKE_CURRENT_SOURCE_DIR}/src")
    set_target_properties(webcface-server-internal PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
    set_target_properties(webcface-encoding PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
    set_target_properties(webcface-message PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
    set_target_properties(webcface-server-store PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
    set_target_properties(webcface PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
endif()
add_library(webcface::webcface ALIAS webcface)

add_library(wcf INTERFACE)
target_include_directories(wcf INTERFACE ${INCLUDE_DIR})
target_compile_features(wcf INTERFACE c_std_99)
if(WEBCFACE_SHARED)
    # sharedの場合はlibc++とリンクする必要もない
    target_link_libraries(wcf INTERFACE $<TARGET_LINKER_FILE:webcface::webcface>)
else()
    target_link_libraries(wcf INTERFACE webcface)
endif()
add_library(webcface::wcf ALIAS wcf)

add_executable(webcface-server ${SVR_MAIN_SRC} ${VERSION_RC})
target_link_libraries(webcface-server PRIVATE
    webcface
    CLI11::CLI11
)
set_target_properties(webcface-server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
target_compile_definitions(webcface-server PRIVATE
    FILE_DESCRIPTION="WebCFace Server"
    FILE_IS_APP
)

if(WEBCFACE_EXAMPLE)
    foreach(SRC IN LISTS EXAMPLE_SRC)
        get_filename_component(TARGET ${SRC} NAME_WE)
        add_executable(webcface-example-${TARGET} ${SRC} ${VERSION_RC})
        target_link_libraries(webcface-example-${TARGET} PRIVATE webcface)
        set(WEBCFACE_EXAMPLE_TARGETS
            ${WEBCFACE_EXAMPLE_TARGETS}
            webcface-example-${TARGET}
        )
        set(WEBCFACE_EXAMPLE_TARGETS_CPACK
            ${WEBCFACE_EXAMPLE_TARGETS_CPACK}
            webcface-example-${TARGET} "Example ${TARGET}"
        )
        set_target_properties(webcface-example-${TARGET} PROPERTIES
            INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
            INSTALL_RPATH_USE_LINK_PATH TRUE
        )
        target_compile_definitions(webcface-example-${TARGET} PRIVATE
            FILE_DESCRIPTION="WebCFace Example ${TARGET}"
            FILE_IS_APP
        )
    endforeach()
    foreach(TARGET image-recv image-send)
        target_link_libraries(webcface-example-${TARGET} PRIVATE
            magickpp
        )
    endforeach()
    if(OpenCV_FOUND)
        foreach(TARGET cv-recv cv-send)
            target_link_libraries(webcface-example-${TARGET} PRIVATE
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
    add_executable(webcface-test ${TEST_SRC})
    target_link_libraries(webcface-test PRIVATE
        webcface-encoding
        webcface-message
        webcface-server-store
        webcface
        GTest::gtest_main
        msgpack-cxx
        utfcpp
        eventpp::eventpp
        ${spdlog}
        ${libcurl}
        ${Crow}
    )
    target_compile_definitions(webcface-test PRIVATE
        WEBCFACE_TEST_TIMEOUT=${WEBCFACE_TEST_TIMEOUT}
    )
    include(GoogleTest)
    gtest_discover_tests(webcface-test DISCOVERY_TIMEOUT 600)
endif()

###############################################################################
if(WEBCFACE_INSTALL)
    include(CMakePackageConfigHelpers)

    # binaries
    install(TARGETS webcface-server
        RUNTIME DESTINATION bin
    )
    install(TARGETS ${WEBCFACE_EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
    )

    # library
    list(APPEND WEBCFACE_EXPORTS webcface wcf)
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
    # private static dependencies
    if(NOT WEBCFACE_SHARED)
        list(APPEND WEBCFACE_EXPORTS libcurl-linker)
        if(NOT CURL_FOUND)
            list(APPEND WEBCFACE_EXPORTS ${libcurl})
            if(NOT WEBCFACE_SHARED)
                if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -lcurl-d)
                else()
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -lcurl)
                endif()
                get_directory_property(CURL_LIBS
                    DIRECTORY ${curl_SOURCE_DIR}
                    DEFINITION CURL_LIBS
                )
                # curlのCMakeListsからコピペ
                foreach(_lib IN LISTS CURL_LIBS)
                    if(TARGET "${_lib}")
                        set(_libname "${_lib}")
                        get_target_property(_imported "${_libname}" IMPORTED)
                        if(NOT _imported)
                            # Reading the LOCATION property on non-imported target will error out.
                            # Assume the user won't need this information in the .pc file.
                            continue()
                        endif()
                        get_target_property(_lib "${_libname}" LOCATION)
                        if(NOT _lib)
                            # message(WARNING "Bad lib in library list: ${_libname}")
                            continue()
                        endif()
                    endif()
                    if(_lib MATCHES ".*/.*" OR _lib MATCHES "^-")
                        list(APPEND WEBCFACE_PKGCONFIG_LIBS ${_lib})
                    else()
                        list(APPEND WEBCFACE_PKGCONFIG_LIBS -l${_lib})
                    endif()
                endforeach()
            endif()
        endif()
        list(APPEND WEBCFACE_EXPORTS magickpp)
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
        EXPORT webcface-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        if(WEBCFACE_SHARED)
            get_target_property(WEBCFACE_PDB webcface PDB_NAME)
        else()
            get_target_property(WEBCFACE_PDB webcface COMPILE_PDB_NAME)
        endif()
        install(FILES ${WEBCFACE_PDB}
            DESTINATION bin
            OPTIONAL
        )
    endif()
    install(EXPORT webcface-targets
        FILE webcface-targets.cmake
        NAMESPACE webcface::
        DESTINATION lib/cmake/webcface
    )
    set(PACKAGE_INIT "@PACKAGE_INIT@")
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/webcface-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/webcface-config.cmake.in
        @ONLY
    )
    unset(PACKAGE_INIT)
    configure_package_config_file(
        ${CMAKE_CURRENT_BINARY_DIR}/webcface-config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/webcface-config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/webcface
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/webcface-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/webcface-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/webcface-config-version.cmake
        DESTINATION lib/cmake/webcface
    )

    # pkg-config
    list(JOIN WEBCFACE_PKGCONFIG_LIBS " " WEBCFACE_PKGCONFIG_LIBS)
    list(JOIN WEBCFACE_PKGCONFIG_REQUIRES " " WEBCFACE_PKGCONFIG_REQUIRES)
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/webcface.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/webcface.pc
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/webcface.pc
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
            ${msgpack-c_SOURCE_DIR}/LICENSE_1_0.txt
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
    if(NOT Crow_FOUND)
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
                ${imagemagick-windows_SOURCE_DIR}/ImageMagick/LICENSE
                DESTINATION share/webcface/3rd_party/ImageMagick
            )
        else()
            install(FILES
                ${imagemagick_SOURCE_DIR}/LICENSE
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
        ament_export_targets(webcface-targets HAS_LIBRARY_TARGET)
        ament_package()
    endif()
endif()

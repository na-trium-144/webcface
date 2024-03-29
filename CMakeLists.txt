cmake_minimum_required(VERSION 3.12)

project(webcface
    LANGUAGES CXX C
    VERSION 1.9.1
    HOMEPAGE_URL "https://github.com/na-trium-144/webcface"
    DESCRIPTION "Web-based RPC & UI Library"
)
set(WEBCFACE_CONTACT "na-trium-144@users.noreply.github.com")
set(WEBCFACE_VERSION ${PROJECT_VERSION})
set(WEBCFACE_SOVERSION 9)
set(WEBUI_VERSION 1.5.0)

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
option(WEBCFACE_USE_OPENCV "use opencv to enable image processing feature" ON)
set(WEBCFACE_VERSION_SUFFIX "git" CACHE STRING
    "version suffix ('git' to get automatically from git describe command)"
)
option(WEBCFACE_DOWNLOAD_WEBUI "download and install webui" ON)
option(WEBCFACE_FIND_LIBS "find and use msgpack-cxx, eventpp and spdlog installation" ON)
option(WEBCFACE_FIND_AMENT "find and use ament_cmake installation" ON)

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

message(STATUS "WebCFace ${WEBCFACE_VERSION} (soversion ${WEBCFACE_SOVERSION})")

message(STATUS "webcface_example = ${WEBCFACE_EXAMPLE}")
message(STATUS "webcface_test = ${WEBCFACE_TEST} (timeout = ${WEBCFACE_TEST_TIMEOUT} ms)")
message(STATUS "webcface_coverage = ${WEBCFACE_COVERAGE}")
message(STATUS "webcface_install = ${WEBCFACE_INSTALL}")
message(STATUS "webcface_install_service = ${WEBCFACE_INSTALL_SERVICE}")
message(STATUS "webcface_shared = ${WEBCFACE_SHARED} (pic = ${WEBCFACE_PIC})")
message(STATUS "webcface_use_opencv = ${WEBCFACE_USE_OPENCV}")
message(STATUS "webcface_find_libs = ${WEBCFACE_FIND_LIBS}")
message(STATUS "webcface_find_ament = ${WEBCFACE_FIND_AMENT}")

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
    add_compile_options(-Wno-psabi) # refer to PR#9
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

configure_file(
    cmake/def.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/include/webcface/common/def.h
    @ONLY
)
configure_file(
    cmake/version.rc.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/version.rc
    @ONLY
)

###############################################################################

include(FetchContent)

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

if(WEBCFACE_FIND_LIBS)
    find_package(msgpack-cxx QUIET)
endif()
if(msgpack-cxx_FOUND)
    message(STATUS "msgpack-cxx ${msgpack-cxx_VERSION} Found: ${msgpack-cxx_DIR}")
else()
    message(STATUS "msgpack-cxx Not Found, Fetching Source...")
    FetchContent_Declare(msgpack-cxx
        GIT_REPOSITORY https://github.com/msgpack/msgpack-c.git
        GIT_TAG cpp-6.1.0
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
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
        target_include_directories(msgpack-cxx INTERFACE
            $<BUILD_INTERFACE:${msgpack-cxx_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        )
    endif()
endif()

if(WEBCFACE_FIND_LIBS)
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
    set(EVENTPP_INSTALL ${WEBCFACE_INSTALL} CACHE INTERNAL "" FORCE)
    FetchContent_MakeAvailable(eventpp)
endif()

if(WEBCFACE_FIND_LIBS)
    find_package(spdlog QUIET)
endif()
if(spdlog_FOUND)
    message(STATUS "spdlog ${spdlog_VERSION} Found: ${spdlog_DIR}")
else()
    message(STATUS "spdlog Not Found, Fetching Source...")
    FetchContent_Declare(spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
    )
    set(SPDLOG_BUILD_SHARED ${WEBCFACE_SHARED} CACHE INTERNAL "" FORCE)
    set(SPDLOG_BUILD_PIC ${WEBCFACE_PIC} CACHE INTERNAL "" FORCE)
    set(SPDLOG_INSTALL ${WEBCFACE_INSTALL} CACHE INTERNAL "" FORCE)
    FetchContent_MakeAvailable(spdlog)
endif()

message(STATUS "Fetching curl Source...")
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
set(BUILD_CURL_EXE off CACHE INTERNAL "" FORCE)
    # libcurlはだいたいインストールされているが使えないのでsharedにする意味がない
set(BUILD_SHARED_LIBS off CACHE INTERNAL "" FORCE)
set(BUILD_STATIC_LIBS on CACHE INTERNAL "" FORCE)
SET(SHARE_LIB_OBJECT ${WEBCFACE_PIC} CACHE INTERNAL "" FORCE)
set(ENABLE_WEBSOCKETS on CACHE INTERNAL "" FORCE)
set(CURL_DISABLE_INSTALL on CACHE INTERNAL "" FORCE)
set(CURL_ENABLE_EXPORT_TARGET off CACHE INTERNAL "" FORCE)
FetchContent_MakeAvailable(curl)

message(STATUS "Fetching asio Source...")
FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-29-0
    CONFIGURE_COMMAND ""
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
    add_library(asio INTERFACE)
    target_include_directories(asio INTERFACE
        ${asio_SOURCE_DIR}/asio/include
    )
    target_compile_definitions(asio INTERFACE ASIO_STANDALONE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        target_compile_options(asio INTERFACE -Wno-deprecated-declarations)
    endif()
    if(MINGW)
        target_link_libraries(asio INTERFACE ws2_32 wsock32)
    endif()
endif()

message(STATUS "Fetching crow Source...")
# asioがない場合find_packageしないようにするため、
# add_subdirectoryではなくinterfaceライブラリを定義する
FetchContent_Declare(crow
    GIT_REPOSITORY https://github.com/CrowCpp/Crow.git
    GIT_TAG 921ce6f
    CONFIGURE_COMMAND ""
)
FetchContent_GetProperties(crow)
if(NOT crow_POPULATED)
    FetchContent_Populate(crow)
    add_library(Crow INTERFACE)
    target_include_directories(Crow INTERFACE
        ${crow_SOURCE_DIR}/include
    )
    target_link_libraries(Crow INTERFACE asio)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_compile_options(Crow INTERFACE -Wno-type-limits)
    endif()
endif()

message(STATUS "Fetching cli11 Source...")
FetchContent_Declare(cli11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.3.2
)
FetchContent_MakeAvailable(cli11)

message(STATUS "Fetching utfcpp Source...")
FetchContent_Declare(utfcpp
    GIT_REPOSITORY https://github.com/nemtrif/utfcpp.git
    GIT_TAG v4.0.5
)
if(NOT utfcpp_POPULATED)
    FetchContent_Populate(utfcpp)
    add_library(utfcpp INTERFACE)
    target_include_directories(utfcpp INTERFACE
        ${utfcpp_SOURCE_DIR}/source
    )
endif()

if(WEBCFACE_USE_OPENCV)
    find_package(OpenCV REQUIRED)
endif()

if(WEBCFACE_TEST)
    if(WEBCFACE_FIND_LIBS)
        find_package(GTest QUIET)
    endif()
    if(GTest_FOUND)
        message(STATUS "GTest ${GTest_VERSION} Found: ${GTest_DIR}")
    else()
        message(STATUS "GTest Not Found, Fetching Source...")
        FetchContent_Declare(googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG v1.14.0
        )
        set(INSTALL_GTEST off CACHE INTERNAL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

###############################################################################
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options(-Wno-psabi) # refer to PR#9
    add_compile_options( # inside opencv
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
    )
    add_compile_options( # inside fmt (fmtlib/fmt#3354)
        -Wno-array-bounds
        -Wno-stringop-overflow
    )
endif()
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    add_compile_options( # inside opencv
        -Wno-c11-extensions
        $<$<COMPILE_LANGUAGE:CXX>:-Wno-deprecated-enum-enum-conversion>
    )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/utf-8 /W4)
endif()

set(CPP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SVR_MAIN_SRC ${CPP_DIR}/server_main.cc)
set(SVR_SRC
    ${CPP_DIR}/server/s_client_data.cc
    ${CPP_DIR}/server/store.cc
    ${CPP_DIR}/server/websock.cc
    ${CPP_DIR}/server/dir.cc
    ${CPP_DIR}/server/ip.cc
)
set(CLI_SRC
    ${CPP_DIR}/client/client.cc
    ${CPP_DIR}/client/client_ws.cc
    ${CPP_DIR}/client/data_store1.cc
    ${CPP_DIR}/client/data_store2.cc
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
set(EXAMPLE_SRC
    ${CPP_DIR}/example/main.cc
    ${CPP_DIR}/example/recv.cc
    ${CPP_DIR}/example/c_example.c
)
if(WEBCFACE_USE_OPENCV)
    set(EXAMPLE_SRC ${EXAMPLE_SRC}
        ${CPP_DIR}/example/image_send.cc
        ${CPP_DIR}/example/image_recv.cc
    )
endif()
set(TEST_SRC
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
    ${CPP_DIR}/test/c_wcf_test.cc
)
set(MSG_SRC
    ${CPP_DIR}/message/message.cc
)
if(WIN32)
    set(VERSION_RC ${CPP_DIR}/version.rc)
else()
    set(VERSION_RC "")
endif()

set(INCLUDE_DIR
    $<BUILD_INTERFACE:${CPP_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

###############################################################################

add_library(${PROJECT_NAME}-headers INTERFACE)
target_include_directories(${PROJECT_NAME}-headers INTERFACE
    ${INCLUDE_DIR}
)
target_compile_features(${PROJECT_NAME}-headers INTERFACE cxx_std_20)
if(WEBCFACE_USE_OPENCV)
    target_link_libraries(${PROJECT_NAME}-headers INTERFACE ${OpenCV_LIBS})
endif()

add_library(${PROJECT_NAME}-message STATIC ${MSG_SRC})
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND WEBCFACE_PIC)
    target_compile_options(${PROJECT_NAME}-message PRIVATE -fPIC)
endif()
target_compile_features(${PROJECT_NAME}-message PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME}-message PUBLIC
    ${PROJECT_NAME}-headers
    msgpack-cxx
    spdlog::spdlog
)
if(MINGW)
    target_link_libraries(${PROJECT_NAME}-message PUBLIC ws2_32)
endif()
if(WIN32)
    set_target_properties(${PROJECT_NAME}-message PROPERTIES
        DEBUG_POSTFIX d
    )
endif()

add_library(${PROJECT_NAME}-server-impl STATIC ${SVR_SRC})
target_link_libraries(${PROJECT_NAME}-server-impl PUBLIC
    ${PROJECT_NAME}-message
    Crow
    utfcpp
)
if(WIN32)
    target_link_libraries(${PROJECT_NAME}-server-impl PRIVATE
        IPHLPAPI.lib
    )
endif()

add_executable(${PROJECT_NAME}-server ${SVR_MAIN_SRC} ${VERSION_RC})
target_link_libraries(${PROJECT_NAME}-server PRIVATE
    ${PROJECT_NAME}-server-impl
    CLI11
)
set_target_properties(${PROJECT_NAME}-server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
target_compile_definitions(${PROJECT_NAME}-server PRIVATE
    FILE_DESCRIPTION="WebCFace Server"
    FILE_IS_APP
)

if(WEBCFACE_SHARED)
    add_library(${PROJECT_NAME} SHARED ${CLI_SRC} ${VERSION_RC})
else()
    add_library(${PROJECT_NAME} STATIC ${CLI_SRC})
endif()
if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" AND WEBCFACE_PIC)
    target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)
endif()
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME} PRIVATE
    ${PROJECT_NAME}-message
    libcurl_static
)
target_link_libraries(${PROJECT_NAME} PUBLIC
    ${PROJECT_NAME}-headers
    eventpp::eventpp
    spdlog::spdlog
)
set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${WEBCFACE_SOVERSION}
)
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        DEBUG_POSTFIX "${WEBCFACE_SOVERSION}d"
        RELEASE_POSTFIX "${WEBCFACE_SOVERSION}"
    )
endif()
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

add_library(wcf INTERFACE)
target_include_directories(wcf INTERFACE ${INCLUDE_DIR})
target_compile_features(wcf INTERFACE c_std_99)
target_link_libraries(wcf INTERFACE
    $<TARGET_LINKER_FILE:${PROJECT_NAME}::${PROJECT_NAME}>
    $<TARGET_LINKER_FILE:spdlog::spdlog>
)
if(WEBCFACE_USE_OPENCV)
    target_link_libraries(wcf INTERFACE ${OpenCV_LIBS})
endif()
add_library(${PROJECT_NAME}::wcf ALIAS wcf)

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
endif()

if(WEBCFACE_TEST)
    enable_testing()
    add_executable(${PROJECT_NAME}-test ${TEST_SRC})
    target_link_libraries(${PROJECT_NAME}-test PRIVATE
        ${PROJECT_NAME}
        ${PROJECT_NAME}-message
        ${PROJECT_NAME}-server-impl
        libcurl_static
        Crow
        GTest::gtest_main
    )
    target_compile_definitions(${PROJECT_NAME}-test PRIVATE
        WEBCFACE_TEST_TIMEOUT=${WEBCFACE_TEST_TIMEOUT}
    )
    include(GoogleTest)
    gtest_discover_tests(${PROJECT_NAME}-test DISCOVERY_TIMEOUT 600)
endif()

###############################################################################
# https://qiita.com/modapone/items/8f97425b6167cffc815c
if(WEBCFACE_INSTALL)
    include(CMakePackageConfigHelpers)

    install(TARGETS ${PROJECT_NAME}-server
        RUNTIME DESTINATION bin
    )
    install(TARGETS ${WEBCFACE_EXAMPLE_TARGETS}
        RUNTIME DESTINATION bin
    )

    set(WEBCFACE_EXPORTS ${PROJECT_NAME} ${PROJECT_NAME}-headers wcf)
    if(NOT WEBCFACE_SHARED)
        set(WEBCFACE_EXPORTS ${WEBCFACE_EXPORTS} ${PROJECT_NAME}-message libcurl_static)
        if(NOT msgpack-cxx_FOUND)
            set(WEBCFACE_EXPORTS ${WEBCFACE_EXPORTS} msgpack-cxx)
        endif()
    endif()
    install(TARGETS ${WEBCFACE_EXPORTS}
        EXPORT ${PROJECT_NAME}-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        install(FILES $<TARGET_PDB_FILE:${PROJECT_NAME}>
            DESTINATION bin
            OPTIONAL
        )
    endif()

    install(EXPORT ${PROJECT_NAME}-targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION lib/cmake/${PROJECT_NAME}
    )
    install(DIRECTORY ${CPP_DIR}/include/
        DESTINATION include
    )
    if(WEBCFACE_USE_OPENCV)
        configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.opencv.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )
    else()
        configure_package_config_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.no-opencv.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )
    endif()

    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        DESTINATION lib/cmake/${PROJECT_NAME}
    )
    if(WEBCFACE_USE_OPENCV)
        set(WEBCFACE_PKGCONFIG_REQUIRES "spdlog opencv4")
    else()
        set(WEBCFACE_PKGCONFIG_REQUIRES "spdlog")
    endif()
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}.pc.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        @ONLY
    )
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION lib/pkgconfig
    )

    if(WEBCFACE_INSTALL_SERVICE)
        install(FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/webcface-server.service
            DESTINATION lib/systemd/system
        )
    endif()

    if(WEBCFACE_DOWNLOAD_WEBUI)
        install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/dist/
            DESTINATION share/webcface/dist
        )
    endif()
    install(FILES
        ${CMAKE_CURRENT_LIST_DIR}/README.md
        ${CMAKE_CURRENT_LIST_DIR}/LICENSE
        DESTINATION share/webcface
    )
    install(FILES
        ${asio_SOURCE_DIR}/asio/LICENSE_1_0.txt
        DESTINATION share/webcface/3rd_party/asio
    )
    install(FILES
        ${cli11_SOURCE_DIR}/LICENSE
        DESTINATION share/webcface/3rd_party/cli11
    )
    # curl: リポジトリ内にないしバイナリ配布に必須ではないのでスキップ
    install(FILES
        ${crow_SOURCE_DIR}/LICENSE
        ${crow_SOURCE_DIR}/README.md
        # 3rdpartyのライセンスがREADMEにある
        DESTINATION share/webcface/3rd_party/crow
    )
    if(NOT msgpack-cxx_FOUND)
        install(FILES
            ${msgpack-cxx_SOURCE_DIR}/LICENSE_1_0.txt
            DESTINATION share/webcface/3rd_party/msgpack-c
        )
    endif()
    install(FILES
        ${utfcpp_SOURCE_DIR}/LICENSE
        DESTINATION share/webcface/3rd_party/utfcpp
    )

    # https://decovar.dev/blog/2021/09/23/cmake-cpack-package-deb-apt/
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
    if(${spdlog_FOUND})
        set(CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} libspdlog-dev)
    endif()
    if(${OpenCV_FOUND})
        set(CPACK_DEBIAN_PACKAGE_DEPENDS ${CPACK_DEBIAN_PACKAGE_DEPENDS} libopencv-dev)
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

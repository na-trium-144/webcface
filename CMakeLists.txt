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
option(WEBCFACE_FIND_OPENCV "try find_package(OpenCV) (only for example)" OFF)
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
endif()

if(MINGW)
    # https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean
    add_link_options(-Wl,--default-image-base-low)
endif()

set(WEBCFACE_DEF_H_PATH include/webcface/common/def.h)
configure_file(
    cmake/def.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/${WEBCFACE_DEF_H_PATH}
    @ONLY
)
set(WEBCFACE_VERSION_RC_PATH version.rc)
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

include(cmake/deps/msgpack-cxx.cmake)
include(cmake/deps/eventpp.cmake)
include(cmake/deps/spdlog.cmake)
include(cmake/deps/curl.cmake)
include(cmake/deps/crow.cmake)
include(cmake/deps/cli11.cmake)
include(cmake/deps/utf8cpp.cmake)
include(cmake/deps/magickpp.cmake)

if(WEBCFACE_FIND_OPENCV)
    find_package(OpenCV QUIET)
endif()
if(OpenCV_FOUND)
    message(STATUS "OpenCV ${OpenCV_VERSION} Found: ${OpenCV_PREFIX}")
else()
    message(STATUS "OpenCV Not Found")
endif()

if(WEBCFACE_TEST)
    include(cmake/deps/googletest.cmake) 
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
    include(cmake/linker.cmake)
    target_static_link(webcface-server-internal-linker
        LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:webcface-server-internal>
        LIBRARIES webcface-server-internal
    )
    add_dependencies(webcface-server-internal-linker webcface-server-internal)
endif()

# encoding: public
add_library(webcface-encoding OBJECT ${WEBCFACE_ENCODING_SRC})
target_include_directories(webcface-encoding PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/encoding/include>)
target_compile_definitions(webcface-encoding PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-encoding PRIVATE utf8cpp)

# message: APIはprivateだがシンボルは公開
add_library(webcface-message OBJECT ${WEBCFACE_MESSAGE_SRC})
target_include_directories(webcface-message PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/message/include>)
target_compile_definitions(webcface-message PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-message PUBLIC
    webcface-encoding
    msgpack-cxx
    utf8cpp
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
    libcurl-linker
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
        libcurl-linker
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
    install(TARGETS webcface-server RUNTIME DESTINATION bin)
    install(TARGETS ${WEBCFACE_EXAMPLE_TARGETS} RUNTIME DESTINATION bin)

    # library
    # dependenciesはそれぞれのcmakeファイルでリストに追加されている
    list(APPEND WEBCFACE_EXPORTS webcface wcf)
    list(APPEND WEBCFACE_PKGCONFIG_LIBS -lwebcface)
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
    install(DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/client/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/encoding/include/
        ${CMAKE_CURRENT_SOURCE_DIR}/server-store/include/
        ${CMAKE_CURRENT_BINARY_DIR}/include/
        DESTINATION include
        PATTERN internal EXCLUDE
    )

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

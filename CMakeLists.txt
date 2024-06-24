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
        message(STATUS "Failed to get version from git describe:\n${git_describe_error}")
    endif()
elseif(NOT WEBCFACE_VERSION_SUFFIX STREQUAL "")
    set(WEBCFACE_VERSION "${PROJECT_VERSION}-${WEBCFACE_VERSION_SUFFIX}")
endif()

if(WEBCFACE_PIC OR CMAKE_POSITION_INDEPENDENT_CODE)
    set(WEBCFACE_PIC ON)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON CACHE BOOL "")
endif()

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

set(output_types RUNTIME ARCHIVE LIBRARY PDB COMPILE_PDB)
foreach(type IN LISTS output_types)
    if("${CMAKE_${type}_OUTPUT_DIRECTORY}" STREQUAL "")
        set(CMAKE_${type}_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    endif()
endforeach()

if(WEBCFACE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        add_link_options(-lgcov --coverage)
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    endif()
endif()

configure_file(
    cmake/def.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/webcface/common/def.h
    @ONLY
)
configure_file(
    cmake/version.rc.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.rc
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
if(WEBCFACE_EXAMPLE)
    include(cmake/deps/opencv.cmake)
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
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
    )
endif()

if(MINGW)
    # https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean
    add_link_options(-Wl,--default-image-base-low)
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

macro(set_target_postfix TARGET)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_target_properties(${TARGET} PROPERTIES
            DEBUG_POSTFIX "${WEBCFACE_SOVERSION}d"
            RELEASE_POSTFIX "${WEBCFACE_SOVERSION}"
        )
    endif()
endmacro()

###############################################################################

add_subdirectory(server-internal)
add_subdirectory(encoding)
add_subdirectory(message)
add_subdirectory(server-store)
add_subdirectory(client)

if(WEBCFACE_SHARED)
    add_library(webcface SHARED)
    if(WIN32)
        target_sources(webcface PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    endif()
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
target_link_libraries(webcface PRIVATE magickpp-linker libcurl-linker)
if(NOT WEBCFACE_SHARED)
    target_link_directories(webcface PUBLIC
        $<TARGET_PROPERTY:magickpp-linker,INTERFACE_LINK_DIRECTORIES>
        $<TARGET_PROPERTY:libcurl-linker,INTERFACE_LINK_DIRECTORIES>
    )
endif()
# public: header only も含む
target_link_libraries(webcface PUBLIC eventpp::eventpp spdlog-linker)
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
set_target_postfix(webcface-server-internal)
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


###############################################################################

add_subdirectory(server)

if(WEBCFACE_EXAMPLE)
    add_subdirectory(examples)
endif()

if(WEBCFACE_TEST)
    add_subdirectory(tests)
endif()

###############################################################################
if(WEBCFACE_INSTALL)
    include(CMakePackageConfigHelpers)

    # binaries
    install(TARGETS webcface-server RUNTIME DESTINATION bin)

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
    get_directory_property(WEBCFACE_EXAMPLE_TARGETS_CPACK
        DIRECTORY examples
        DEFINITION WEBCFACE_EXAMPLE_TARGETS_CPACK
    )
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

###############################################################################

message(STATUS "** WebCFace ${WEBCFACE_VERSION} (soversion: ${WEBCFACE_SOVERSION}) Summary **")
message(STATUS " CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS " WEBCFACE_CONFIG_ALL = ${WEBCFACE_CONFIG_ALL}")
endif()
message(STATUS " WEBCFACE_SHARED = ${WEBCFACE_SHARED} (PIC = ${WEBCFACE_PIC})")
message(STATUS " WEBCFACE_EXAMPLE = ${WEBCFACE_EXAMPLE}")
if(WEBCFACE_TEST)
    message(STATUS " WEBCFACE_TEST = ${WEBCFACE_TEST} (TIMEOUT = ${WEBCFACE_TEST_TIMEOUT} ms)")
    # message(STATUS " WEBCFACE_COVERAGE = ${WEBCFACE_COVERAGE}")
else()
    message(STATUS " WEBCFACE_TEST = ${WEBCFACE_TEST}")
endif()
if(WEBCFACE_INSTALL)
    message(STATUS " WEBCFACE_INSTALL = ${WEBCFACE_INSTALL} (SERVICE = ${WEBCFACE_INSTALL_SERVICE})")
else()
    message(STATUS " WEBCFACE_INSTALL = ${WEBCFACE_INSTALL}")
endif()
if(WEBCFACE_DOWNLOAD_WEBUI)
    message(STATUS " WEBCFACE_DOWNLOAD_WEBUI = ${WEBCFACE_DOWNLOAD_WEBUI} (v${WEBUI_VERSION} at ${CMAKE_CURRENT_BINARY_DIR}/dist)")
else()
    message(STATUS " WEBCFACE_DOWNLOAD_WEBUI = ${WEBCFACE_DOWNLOAD_WEBUI}")
endif()
message(STATUS " Dependencies:")
foreach(status IN LISTS WEBCFACE_SUMMARY)
    message(STATUS " - ${status}")
endforeach()

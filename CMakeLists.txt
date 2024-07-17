cmake_minimum_required(VERSION 3.14)
# ubuntu20.04: cmake 3.16.3

set(ORIGINAL_ENV_CC "$ENV{CC}" CACHE STRING "")
set(ORIGINAL_ENV_CXX "$ENV{CXX}" CACHE STRING "")

if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
    cmake_policy(SET CMP0135 NEW)
endif()

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
option(WEBCFACE_FRAMEWORK "build webcface framework bundle" OFF)
option(WEBCFACE_EXAMPLE "build example" ${IS_MAIN})
option(WEBCFACE_TEST "build test" OFF)
option(WEBCFACE_COVERAGE "enable generating test coverage" OFF)
set(WEBCFACE_TEST_TIMEOUT 10 CACHE STRING "connection timeout in test")
option(WEBCFACE_SHARED "build shared library" ON)
option(WEBCFACE_PIC "enable PIC for shared library" ${WEBCFACE_SHARED})
set(WEBCFACE_VERSION_SUFFIX "git" CACHE STRING "version suffix ('git' to get automatically from git describe command)")
set(WEBCFACE_CLANG_TIDY "" CACHE STRING "clang-tidy path (or empty to disable clang-tidy)")
option(WEBCFACE_PERMISSIVE "add -fpermissive" OFF)
option(WEBCFACE_WARNING "enable all warning and treat most of them as error" ${IS_MAIN})
option(WEBCFACE_DOWNLOAD_WEBUI "download and install webui" ${IS_MAIN})
option(WEBCFACE_CONFIG_ALL "configure both debug and release build on windows" OFF)
option(WEBCFACE_FIND_LIBS "if false, disables every find_package and pkg_check_modules" ${WEBCFACE_SHARED})
option(WEBCFACE_FIND_AMENT "try find_package(ament_cmake)" ON)
option(WEBCFACE_SERVER "build server" ON)

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

include(cmake/system.cmake)
include(cmake/filesystem.cmake)

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

###############################################################################

include(FetchContent)

if(WEBCFACE_DOWNLOAD_WEBUI)
    FetchContent_Declare(webui
        URL https://github.com/na-trium-144/webcface-webui/releases/download/v${WEBUI_VERSION}/webcface-webui_${WEBUI_VERSION}.tar.gz
        SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/dist
    )
    FetchContent_MakeAvailable(webui)
endif()

if(WEBCFACE_FIND_AMENT)
    find_package(ament_cmake QUIET)
endif()
if(ament_cmake_FOUND)
    message(STATUS "ament_cmake Found")
endif()

include(cmake/deps/msgpack-cxx.cmake)
include(cmake/deps/spdlog.cmake)
include(cmake/deps/curl.cmake)
include(cmake/deps/cli11.cmake)
include(cmake/deps/utf8cpp.cmake)
if(WEBCFACE_SERVER)
    include(cmake/deps/crow.cmake)
    include(cmake/deps/magickpp.cmake)
else()
    add_library(crow-linker INTERFACE)
    add_library(magickpp-linker INTERFACE)
    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS magickpp-linker)
    endif()
endif()
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
    if(WEBCFACE_WARNING AND NOT WEBCFACE_PERMISSIVE)
        add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    endif()
    if(WEBCFACE_PERMISSIVE)
        add_compile_options(-fpermissive)
    endif()
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
    if(WEBCFACE_WARNING)
        add_compile_options(-Wall -Wextra -Wpedantic -Werror)
    endif()
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
    if(WEBCFACE_WARNING)
        add_compile_options(/W4 /wd4251 /wd4275 /wd4661 /wd4996 /WX)
        string(REPLACE "/W3" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    endif()
    add_compile_options(/external:anglebrackets /external:W0)
    add_compile_options(/utf-8 /bigobj)
    add_compile_definitions(
        _CRT_SECURE_NO_WARNINGS
        _SILENCE_STDEXT_ARR_ITERS_DEPRECATION_WARNING
    )
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

###############################################################################

add_subdirectory(server-internal)
add_subdirectory(encoding)
add_subdirectory(message)
add_subdirectory(server-store)
add_subdirectory(client)

set(WEBCFACE_TARGETS
    webcface-server-internal
    webcface-encoding
    webcface-message
    webcface-server-store
    webcface-client
)

if(WEBCFACE_SHARED)
    add_library(webcface SHARED)
    if(WEBCFACE_SYSTEM_VERSION_RC)
        target_sources(webcface PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
    endif()
else()
    add_library(webcface STATIC)
endif()
target_compile_features(webcface PUBLIC cxx_std_17)
target_link_libraries(webcface PUBLIC webcface-std-filesystem)
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
if(WEBCFACE_SERVER)
    if(WEBCFACE_SHARED)
        target_link_libraries(webcface PRIVATE webcface-server-internal-linker)
    else()
        target_sources(webcface PRIVATE $<TARGET_OBJECTS:webcface-server-internal>)
    endif()
endif()
# private: リンクが必要なもののみ
target_link_libraries(webcface PRIVATE
    magickpp-linker
    libcurl-linker
    spdlog-linker
)
if(NOT WEBCFACE_SHARED)
    target_link_directories(webcface PUBLIC
        $<TARGET_PROPERTY:magickpp-linker,INTERFACE_LINK_DIRECTORIES>
        $<TARGET_PROPERTY:libcurl-linker,INTERFACE_LINK_DIRECTORIES>
        $<TARGET_PROPERTY:spdlog-linker,INTERFACE_LINK_DIRECTORIES>
    )
endif()
if(WEBCFACE_SYSTEM_WIN32SOCKET)
    target_link_libraries(webcface PRIVATE iphlpapi)
endif()
if(WEBCFACE_SHARED AND WEBCFACE_SYSTEM_VISIBILITY)
    # webcfaceのAPI以外をexportしない。
    # MSVCではデフォルトでdllexportしたもの以外exportされないので問題なし
    foreach(target IN LISTS WEBCFACE_TARGETS)
        set_target_properties(${target} PROPERTIES
            CXX_VISIBILITY_PRESET hidden
            VISIBILITY_INLINES_HIDDEN 1
        )
    endforeach()
    if(WEBCFACE_SYSTEM_EXCLUDE_LIBS)
        # object libraryのwebcface-messageなどはexcludeされない
        target_link_options(webcface PRIVATE -Wl,--exclude-libs,ALL)
    endif()
endif()
if(WEBCFACE_SYSTEM_ADD_VERSION)
    set_target_properties(webcface PROPERTIES RUNTIME_OUTPUT_NAME "webcface${WEBCFACE_SOVERSION}")
endif()
if(WEBCFACE_SYSTEM_ADD_DEBUG)
    set_target_properties(webcface PROPERTIES DEBUG_POSTFIX "d")
endif()
set_target_properties(webcface PROPERTIES
    INSTALL_RPATH "$ORIGIN/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
    VERSION ${PROJECT_VERSION}
    SOVERSION ${WEBCFACE_SOVERSION}
)
if(WEBCFACE_FRAMEWORK)
    # https://stackoverflow.com/questions/71006897/how-to-preserve-directory-layout-of-public-headers-when-generating-macos-framewo
    file(GLOB_RECURSE WEBCFACE_HEADERS
        encoding/include/*.h
        server-store/include/*.h
        client/include/*.h
        ${CMAKE_CURRENT_BINARY_DIR}/include/*.h
    )
    list(FILTER WEBCFACE_HEADERS EXCLUDE REGEX "internal")
    foreach(_file IN LISTS WEBCFACE_HEADERS)
        get_filename_component(_loc "${_file}" DIRECTORY)
        string(REGEX REPLACE ".*/include/webcface" "Headers" _loc "${_loc}")
        set_source_files_properties(${_file} PROPERTIES MACOSX_PACKAGE_LOCATION ${_loc})
        message("${_file}: ${_loc}")
    endforeach()
    target_sources(webcface PRIVATE ${WEBCFACE_HEADERS})
    set_target_properties(webcface PROPERTIES
        FRAMEWORK TRUE
        FRAMEWORK_VERSION ${WEBCFACE_SOVERSION}
        MACOSX_FRAMEWORK_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_FRAMEWORK_IDENTIFIER "com.github.na-trium-144.webcface"
        MACOSX_FRAMEWORK_SHORT_VERSION_STRING "${WEBCFACE_VERSION}"
    )
endif()
if(NOT "${WEBCFACE_CLANG_TIDY}" STREQUAL "")
    set(CLANG_TIDY_CMD "${WEBCFACE_CLANG_TIDY}" "--header-filter=${CMAKE_CURRENT_SOURCE_DIR}/src")
    foreach(target IN LISTS WEBCFACE_TARGETS)
        set_target_properties(${target} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
    endforeach()
endif()
add_library(webcface::webcface ALIAS webcface)

add_library(wcf INTERFACE)
target_include_directories(wcf INTERFACE $<TARGET_PROPERTY:webcface,INTERFACE_INCLUDE_DIRECTORIES>)
target_compile_features(wcf INTERFACE c_std_99)
if(WEBCFACE_SHARED)
    # sharedの場合はlibc++とリンクする必要もない
    target_link_libraries(wcf INTERFACE $<TARGET_LINKER_FILE:webcface::webcface>)
else()
    target_link_libraries(wcf INTERFACE webcface)
endif()
add_library(webcface::wcf ALIAS wcf)

###############################################################################

if(WEBCFACE_SERVER)
    add_subdirectory(server)
endif()

if(WEBCFACE_EXAMPLE)
    add_subdirectory(examples)
endif()

if(WEBCFACE_TEST)
    enable_testing()
    add_subdirectory(tests)
endif()

###############################################################################
if(WEBCFACE_INSTALL)
    include(CMakePackageConfigHelpers)

    # binaries
    if(WEBCFACE_SERVER)
        install(TARGETS webcface-server RUNTIME DESTINATION bin)
    endif()

    # library
    # dependenciesはそれぞれのcmakeファイルでリストに追加されている
    # cmake時は依存ライブラリを先に処理するが、linker flagは逆で依存ライブラリがあと
    list(APPEND WEBCFACE_EXPORTS webcface wcf webcface-std-filesystem)
    list(INSERT WEBCFACE_PKGCONFIG_CFLAGS 0 "-I\${includedir}")
    list(INSERT WEBCFACE_PKGCONFIG_LIBS 0 "-L\${libdir}" -lwebcface)
    install(TARGETS ${WEBCFACE_EXPORTS}
        EXPORT webcface-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        FRAMEWORK DESTINATION Frameworks
    )

    # pdbがあればインストールする
    if(WEBCFACE_SHARED)
        get_target_property(WEBCFACE_PDB webcface PDB_NAME)
    else()
        get_target_property(WEBCFACE_PDB webcface COMPILE_PDB_NAME)
    endif()
    install(FILES ${WEBCFACE_PDB}
        DESTINATION bin
        OPTIONAL
    )

    if(WEBCFACE_FRAMEWORK)
        # 一応書いたけど、pkgconfig内でフラグにスペースが含まれる時、
        # bashで展開しようとするとうまくエスケープされないので使い物にならない気がする...
        set(WEBCFACE_PKGCONFIG_CFLAGS
            "-F\${prefix}/Frameworks"
            "-F/Applications/WebCFace\\ Desktop/Contents/Frameworks"
        )
        set(WEBCFACE_PKGCONFIG_LIBS
            ${WEBCFACE_PKGCONFIG_CFLAGS}
            -framework webcface
        )
        configure_file(
            ${CMAKE_CURRENT_SOURCE_DIR}/cmake/webcface-config.framework.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/webcface-config.cmake
            COPYONLY
        )
    else()
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
    endif()
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
    list(JOIN WEBCFACE_PKGCONFIG_CFLAGS " " WEBCFACE_PKGCONFIG_CFLAGS)
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
    if(WEBCFACE_EXAMPLE)
        get_directory_property(WEBCFACE_EXAMPLE_TARGETS_CPACK
            DIRECTORY examples
            DEFINITION WEBCFACE_EXAMPLE_TARGETS_CPACK
        )
    endif()
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
if(APPLE)
    message(STATUS " WEBCFACE_FRAMEWORK = ${WEBCFACE_FRAMEWORK}")
    message(STATUS " CMAKE_OSX_ARCHITECTURES = ${CMAKE_OSX_ARCHITECTURES}")
endif()
message(STATUS " WEBCFACE_SERVER = ${WEBCFACE_SERVER}")
message(STATUS " WEBCFACE_EXAMPLE = ${WEBCFACE_EXAMPLE}")
if(WEBCFACE_TEST)
    message(STATUS " WEBCFACE_TEST = ${WEBCFACE_TEST} (TIMEOUT = ${WEBCFACE_TEST_TIMEOUT} ms)")
    # message(STATUS " WEBCFACE_COVERAGE = ${WEBCFACE_COVERAGE}")
else()
    message(STATUS " WEBCFACE_TEST = ${WEBCFACE_TEST}")
endif()
if(WEBCFACE_INSTALL)
    message(STATUS " WEBCFACE_INSTALL = ${WEBCFACE_INSTALL} (SERVICE = ${WEBCFACE_INSTALL_SERVICE})")
    message(STATUS " CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")
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

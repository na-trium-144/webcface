cmake_minimum_required(VERSION 3.5)

project(webcface VERSION 3.2.0)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(IS_MAIN on)
else()
    set(IS_MAIN off)
endif()

option(WEBCFACE_INSTALL "generate install target" ${IS_MAIN})
option(WEBCFACE_SERVER "build server" ${IS_MAIN})
option(WEBCFACE_EXAMPLE "build example" ${IS_MAIN})
# option(WEBCFACE_TEST "build test" OFF)
# set(WEBCFACE_TEST_TIMEOUT 10 CACHE STRING "connection timeout in test")
# option(WEBCFACE_SHARED "build shared library" ON)
option(WEBCFACE_PIC "enable PIC for shared library" ON)
set(WEBCFACE_VERSION_SUFFIX "git" CACHE STRING "version suffix ('git' to get automatically from git describe command)")
option(WEBCFACE_DOWNLOAD_WEBUI "download and install webui" ${IS_MAIN})
option(WEBCFACE_FIND_LIBS "if false, disables searching dependency" OFF)
# option(WEBCFACE_FIND_AMENT "try find_package(ament_cmake)" ON)
set(WEBCFACE_FORCE_FALLBACK_FOR "libcurl,Crow" CACHE STRING "comma separated list of dependencies to force fallback")

if(NOT WEBCFACE_SERVER)
    list(APPEND SETUP_ARG "-Dserver=disabled")
endif()
if(NOT WEBCFACE_EXAMPLE)
    list(APPEND SETUP_ARG "-Dexamples=disabled")
endif()
list(APPEND SETUP_ARG "-Dtests=disabled")

# if(NOT WEBCFACE_SHARED)
#     list(APPEND SETUP_ARG "-Ddefault_library=static")
# endif()
if(NOT WEBCFACE_PIC AND NOT CMAKE_POSITION_INDEPENDENT_CODE)
    list(APPEND SETUP_ARG "-Db_pic=false")
endif()
list(APPEND SETUP_ARG "-Dversion_suffix=${WEBCFACE_VERSION_SUFFIX}")
if(NOT WEBCFACE_DOWNLOAD_WEBUI)
    list(APPEND SETUP_ARG "-Ddownload_webui=disabled")
endif()
if(WEBCFACE_FIND_LIBS)
    list(APPEND SETUP_ARG "-Dwrap_mode=default")
    list(APPEND SETUP_ARG "--force-fallback-for=${WEBCFACE_FORCE_FALLBACK_FOR}")
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND SETUP_ARG "--buildtype=debug")
endif()
list(APPEND SETUP_ARG "--prefix=${CMAKE_INSTALL_PREFIX}")
list(APPEND SETUP_ARG "-Dcmake_prefix_path=${CMAKE_PREFIX_PATH}")

# if(WEBCFACE_FIND_AMENT)
#     find_package(ament_cmake QUIET)
# endif()
# if(ament_cmake_FOUND)
#     message(STATUS "ament_cmake Found")
# endif()

find_program(MESON_COMMAND meson)
if(MESON_COMMAND STREQUAL "MESON_COMMAND-NOTFOUND")
    message(FATAL_ERROR "meson command not found")
endif()
execute_process(
    COMMAND "${MESON_COMMAND}" setup "${CMAKE_CURRENT_BINARY_DIR}/meson"
        "--reconfigure" ${SETUP_ARG}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    RESULT_VARIABLE SETUP_RESULT
)
if(NOT SETUP_RESULT STREQUAL 0)
    message(FATAL_ERROR "meson setup failed")
endif()

add_custom_target(webcface-build ALL
    COMMAND "${MESON_COMMAND}" compile
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/meson"
    COMMENT "building webcface with meson compile"
    USES_TERMINAL
    VERBATIM
)

foreach(target webcface wcf)
    add_library(${target} INTERFACE)
    add_dependencies(${target} webcface-build)
    target_compile_definitions(${target} INTERFACE $<BUILD_INTERFACE:WEBCFACE_MESON>)
    target_include_directories(${target} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/common/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/client/include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/meson>
    )
    target_link_directories(${target} INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/meson>
    )
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_link_libraries(${target} INTERFACE
            debug webcfaced.lib
            optimized webcface.lib
        )
    else()
        target_link_libraries(${target} INTERFACE -lwebcface)
    endif()
endforeach()
target_compile_features(webcface INTERFACE cxx_std_17)
target_compile_features(wcf INTERFACE c_std_99)

add_library(webcface::webcface ALIAS webcface)
add_library(webcface::wcf ALIAS wcf)

if(WEBCFACE_INSTALL)
    install(CODE
        "execute_process(
            COMMAND \"${MESON_COMMAND}\" install --skip-subprojects
            WORKING_DIRECTORY \"${CMAKE_CURRENT_BINARY_DIR}/meson\"
        )"
    )

    # if(ament_cmake_FOUND)
    #     ament_export_targets(webcface-targets HAS_LIBRARY_TARGET)
    #     ament_package()
    # endif()
endif()


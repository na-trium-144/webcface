cmake_minimum_required(VERSION 3.17)
project(webcface
    VERSION 0.9.6
    LANGUAGES CXX
)

if(
    (NOT "${CMAKE_CURRENT_LIST_DIR}" STREQUAL "${CMAKE_SOURCE_DIR}") AND 
    ("${WEBCFACE_LEGACY}" STREQUAL "" OR "${WEBCFACE_LEGACY}")
)
    message("WebCFace Legacy: 旧バージョンのCMakeListsを使用します")
    message("新バージョンを使うには set(WEBCFACE_LEGACY false) してください")
    include(cmake/legacy/CMakeLists.txt)
else()
message("WebCFace 新バージョンのCMakeListsを使用します")

message("${PROJECT_NAME} ${CMAKE_PROJECT_VERSION}")

set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()

set(WEBCFACE_CONFIG_PATH "/tmp/webcface_drogon_config.json")

if("${WEBCFACE_BUILD_FRONT}" STREQUAL "")
    set(WEBCFACE_BUILD_FRONT on)
endif()

file(GLOB MAIN_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/src/lib/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/src/controllers/*.cc
  ${CMAKE_CURRENT_LIST_DIR}/src/pybind/*.cpp
)


add_library(${PROJECT_NAME} SHARED)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_compile_options(${PROJECT_NAME} PUBLIC -fPIC)

# c++20以上が必要(concepts)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set(CMAKE_CXX_STANDARD 20) # ↑だけで十分なはずだが、ないとえらーになる drogon側でなんかやってるのかな?

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    WEBCFACE_VERSION="${PROJECT_VERSION}"
    WEBCFACE_CONFIG_PATH="${WEBCFACE_CONFIG_PATH}"
)

set(WEBCFACE_STATIC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/frontend/out)
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:WEBCFACE_STATIC_DIR="${CMAKE_CURRENT_LIST_DIR}/frontend/out">
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    INSTALL_RPATH "$ORIGIN/;"
    INSTALL_RPATH_USE_LINK_PATH TRUE
    VERSION ${PROJECT_VERSION}
)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon
# add_subdirectory(drogon)
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
find_package(Drogon CONFIG)
if(${Drogon_FOUND})
    set(DROGON Drogon::Drogon)
else()
    set(BUILD_SHARED_LIBS on CACHE INTERNAL "" FORCE)
    add_subdirectory(external/drogon)
    set(DROGON drogon)
endif()
find_package(jsoncpp REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC jsoncpp ${DROGON})

target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/inc>
    $<INSTALL_INTERFACE:include>
)
target_sources(${PROJECT_NAME} PRIVATE ${MAIN_SOURCES})

# ##############################################################################

# ここに書いても動かん
# set(WEBCFACE_GENERATOR_COMMAND ${CMAKE_CURRENT_LIST_DIR}/generator/main.py)
include(cmake/generate.cmake)
include(cmake/frontend.cmake)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    include(cmake/pybind.cmake)
    if(${WEBCFACE_BUILD_FRONT})
        include(cmake/install.cmake)
    endif()
    include(cmake/examples.cmake)
    include(cmake/test.cmake)
endif()

endif()

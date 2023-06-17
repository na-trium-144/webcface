set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "") # works

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-fdiagnostics-color=always)
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    add_compile_options(-fcolor-diagnostics)
endif()

if("${WEB_ROOT_DIR}" STREQUAL "")
    set(WEB_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
else()
    message("webcface root directory = ${WEB_ROOT_DIR}")
endif()
set(WEB_STATIC_DIR_DEFAULT ${WEB_ROOT_DIR}/frontend/out)
set(WEB_CONFIG_PATH "/tmp/webcface_drogon_config.json")

if("${WEB_BUILD_FRONT}" STREQUAL "")
    set(WEB_BUILD_FRONT on)
endif()

file(GLOB MAIN_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/lib/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/controllers/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/src/pybind/*.cpp
)


add_library(${PROJECT_NAME} SHARED)
target_compile_options(${PROJECT_NAME} PUBLIC -fPIC)

# c++20以上が必要(concepts)
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
set(CMAKE_CXX_STANDARD 20) # ↑だけで十分なはずだが、ないとえらーになる drogon側でなんかやってるのかな?

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    WEBCFACE_VERSION="${PROJECT_VERSION}"
    WEBCFACE_CONFIG_PATH="${WEB_CONFIG_PATH}"
)
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    WEBCFACE_STATIC_DIR="${WEB_STATIC_DIR_DEFAULT}"
)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon
# add_subdirectory(drogon)
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
find_package(Drogon CONFIG REQUIRED)
find_package(jsoncpp REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC jsoncpp Drogon::Drogon)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc)
target_sources(${PROJECT_NAME} PRIVATE ${MAIN_SOURCES})

# ##############################################################################
include(cmake/legacy/generate.cmake)
include(cmake/frontend.cmake)

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    include(cmake/pybind.cmake)
    include(cmake/examples.cmake)
    include(cmake/test.cmake)
endif()

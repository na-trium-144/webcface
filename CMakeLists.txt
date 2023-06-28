cmake_minimum_required(VERSION 3.5)
project(webcface CXX)

add_compile_options(-g)

# c++20 for concepts
set(CMAKE_CXX_STANDARD 20)

find_package(Drogon CONFIG)
if(${Drogon_FOUND})
    set(DROGON Drogon::Drogon)
else()
    set(BUILD_CTL off CACHE INTERNAL "" FORCE)
    set(BUILD_EXAMPLES off CACHE INTERNAL "" FORCE)
    set(BUILD_ORM off CACHE INTERNAL "" FORCE)
    set(BUILD_SHARED_LIBS on CACHE INTERNAL "" FORCE)
    set(BUILD_BROTLI off CACHE INTERNAL "" FORCE)
    set(BUILD_YAML_CONFIG off CACHE INTERNAL "" FORCE)
    add_subdirectory(external/drogon)
    set(DROGON drogon)
endif()

find_package(msgpack CONFIG)
if(NOT ${msgpack_FOUND})
    set(MSGPACK_BUILD_DOCS off CACHE INTERNAL "" FORCE)
    set(MSGPACK_BUILD_TESTS off CACHE INTERNAL "" FORCE)
    set(MSGPACK_BUILD_EXAMPLES off CACHE INTERNAL "" FORCE)
    set(MSGPACK_USE_BOOST off CACHE INTERNAL "" FORCE)
    add_subdirectory(external/msgpack-c)
endif()
set(MSGPACK msgpack-cxx)

enable_testing() # todo: オプションで変えられるようにする

aux_source_directory(cpp/server SVR_SRC)
aux_source_directory(cpp/client CLI_SRC)
aux_source_directory(cpp/example EXAMPLE_SRC)
aux_source_directory(cpp/test TEST_SRC)
aux_source_directory(cpp/message MSG_SRC)
aux_source_directory(cpp/controllers CTL_SRC)
# aux_source_directory(cpp/filters FILTER_SRC)
# aux_source_directory(cpp/plugins PLUGIN_SRC)
# aux_source_directory(cpp/models MODEL_SRC)

###############################################################################
# server

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SERVER ${PROJECT_NAME}_server)
add_executable(${SERVER} ${SVR_SRC} ${MSG_SRC} ${CTL_SRC})

target_compile_features(${SERVER} PUBLIC cxx_std_17)
target_link_libraries(${SERVER}
    PRIVATE
    ${DROGON}
    ${MSGPACK}
)

# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR})
# use the following line to create views with namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE)
# use the following line to create views with namespace CHANGE_ME prefixed
# and path namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE CHANGE_ME)

# ##############################################################################
# uncomment the following line for dynamically loading views 
# set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

# ##############################################################################

# client

add_library(${PROJECT_NAME} SHARED ${CLI_SRC} ${MSG_SRC})
target_compile_options(${PROJECT_NAME} PRIVATE -fPIC)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    ${DROGON}
    ${MSGPACK}
)

target_include_directories(${PROJECT_NAME}
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include
)

foreach(SRC IN LISTS EXAMPLE_SRC)
    get_filename_component(TARGET ${SRC} NAME_WE)
    add_executable(example_${TARGET} ${SRC})
    target_link_libraries(example_${TARGET} PRIVATE ${PROJECT_NAME})
endforeach()

foreach(SRC IN LISTS TEST_SRC)
    get_filename_component(TARGET ${SRC} NAME_WE)
    add_executable(test_${TARGET} ${SRC})
    target_link_libraries(test_${TARGET} PRIVATE webcface ${DROGON})
    ParseAndAddDrogonTests(test_${TARGET})
endforeach()


# server-internal: private
if(WEBCFACE_SHARED)
    # Crowとasioをwebcfaceのインタフェースから隔離するためにライブラリを分ける
    # (こうしないとLinuxでどうしてもasioのシンボルがwebcfaceに残ってセグフォする)
    add_library(webcface-server-internal STATIC)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set_target_properties(webcface-server-internal PROPERTIES DEBUG_POSTFIX "d")
    endif()
else()
    add_library(webcface-server-internal OBJECT)
endif()
target_sources(webcface-server-internal PRIVATE
    src/server_ws.cc
)
target_include_directories(webcface-server-internal PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_link_libraries(webcface-server-internal PRIVATE crow-linker)

if(WEBCFACE_SHARED)
    add_library(webcface-server-internal-linker INTERFACE)
    include(../cmake/linker.cmake)
    target_static_link(webcface-server-internal-linker
        BUILD_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:webcface-server-internal>
        LIBRARIES webcface-server-internal
    )
    add_dependencies(webcface-server-internal-linker webcface-server-internal)
endif()

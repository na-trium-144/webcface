# server-internal: private
if(WEBCFACE_SERVER)
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
    target_link_libraries(webcface-server-internal PRIVATE webcface-crow-linker)
    target_compile_features(webcface-server-internal PRIVATE cxx_std_17)
else()
    # build nothing
    add_library(webcface-server-internal INTERFACE)
    target_include_directories(webcface-server-internal INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
endif()

if(WEBCFACE_SHARED AND WEBCFACE_SERVER)
    add_library(webcface-server-internal-linker INTERFACE)
    include(../cmake/linker.cmake)
    target_static_link(webcface-server-internal-linker
        BUILD_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:webcface-server-internal>
        DEBUG_LIBRARIES webcface-server-internal
        RELEASE_LIBRARIES webcface-server-internal
    )
    add_dependencies(webcface-server-internal-linker webcface-server-internal)
endif()

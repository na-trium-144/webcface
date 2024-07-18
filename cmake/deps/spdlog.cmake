include(cmake/fetch.cmake)
option(WEBCFACE_FIND_SPDLOG "try find_package(spdlog)" ${WEBCFACE_FIND_LIBS})

# target = webcface-spdlog-linker

if(WEBCFACE_FIND_SPDLOG)
    find_package(spdlog QUIET)
endif()
if(spdlog_FOUND)
    list(APPEND WEBCFACE_SUMMARY "spdlog: ${spdlog_VERSION} found at ${spdlog_DIR}")
    add_library(webcface-spdlog-linker INTERFACE)
    target_link_libraries(webcface-spdlog-linker INTERFACE spdlog::spdlog)

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-spdlog-linker)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES spdlog)
        endif()
        set(SPDLOG_INSTALLED 1)
    endif()
else()
    set(SPDLOG_INSTALL OFF CACHE INTERNAL "" FORCE)
    fetch_cmake(spdlog
        https://github.com/gabime/spdlog.git
        v1.12.0
    )
    include(cmake/linker.cmake)
    add_library(webcface-spdlog-linker INTERFACE)
    target_include_directories(webcface-spdlog-linker INTERFACE
        $<BUILD_INTERFACE:$<TARGET_PROPERTY:spdlog,INTERFACE_INCLUDE_DIRECTORIES>>
    )
    target_compile_definitions(webcface-spdlog-linker INTERFACE
        $<BUILD_INTERFACE:$<TARGET_PROPERTY:spdlog,INTERFACE_COMPILE_DEFINITIONS>>
    )
    target_static_link(webcface-spdlog-linker
        BUILD_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:spdlog>
        INSTALL_LIBRARY_DIRS $<TARGET_LINKER_FILE_DIR:webcface::spdlog>
        DEBUG_LIBRARIES spdlogd
        RELEASE_LIBRARIES spdlog
    )
    add_dependencies(webcface-spdlog-linker spdlog)

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-spdlog-linker)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_EXPORTS spdlog)
            if(CMAKE_BUILD_TYPE STREQUAL "Debug")
                list(INSERT WEBCFACE_PKGCONFIG_LIBS 0 -lspdlogd)
            else()
                list(INSERT WEBCFACE_PKGCONFIG_LIBS 0 -lspdlog)
            endif()
        endif()
        set(SPDLOG_INSTALLED 0)
        install(FILES
            ${spdlog_SOURCE_DIR}/LICENSE
            DESTINATION share/webcface/3rd_party/spdlog
        )
    endif()
endif()

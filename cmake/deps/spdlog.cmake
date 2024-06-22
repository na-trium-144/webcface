include(cmake/fetch.cmake)
option(WEBCFACE_FIND_SPDLOG "try find_package(spdlog)" ${WEBCFACE_FIND_LIBS})

# target = spdlog-linker

if(WEBCFACE_FIND_SPDLOG)
    find_package(spdlog QUIET)
endif()
if(spdlog_FOUND)
    message(STATUS "spdlog ${spdlog_VERSION} Found: ${spdlog_DIR}")
    add_library(spdlog-linker INTERFACE)
    target_link_libraries(spdlog-linker INTERFACE spdlog::spdlog)

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS spdlog-linker)
        list(APPEND WEBCFACE_PKGCONFIG_REQUIRES spdlog)
        set(SPDLOG_INSTALLED 1)
    endif()
else()
    message(STATUS "spdlog Not Found")
    set(SPDLOG_INSTALL OFF CACHE INTERNAL "" FORCE)
    if(WIN32)
        set(SPDLOG_WCHAR_SUPPORT ON CACHE INTERNAL "" FORCE)
    endif()
    fetch_cmake(spdlog
        https://github.com/gabime/spdlog.git
        v1.12.0
    )
    add_library(spdlog-linker INTERFACE)
    target_link_libraries(spdlog-linker INTERFACE spdlog_header_only)

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS spdlog-linker spdlog_header_only)
        set(SPDLOG_INSTALLED 0)
        install(DIRECTORY ${spdlog_SOURCE_DIR}/include/ DESTINATION include)
        install(FILES
            ${spdlog_SOURCE_DIR}/LICENSE
            DESTINATION share/webcface/3rd_party/spdlog
        )
    endif()
endif()

include(cmake/fetch.cmake)
option(WEBCFACE_FIND_ZLIB "try find_package(zlib) (while building zlib)" ${WEBCFACE_FIND_LIBS})

# target = zlib
unset(zlib_FOUND CACHE)
if(WEBCFACE_FIND_ZLIB)
    pkg_check_modules(zlib QUIET zlib)
endif()
if(zlib_FOUND)
    message(STATUS "zlib ${zlib_VERSION} Found: ${zlib_PREFIX}")
    add_library(zlib INTERFACE)
    target_link_directories(zlib INTERFACE ${zlib_LIBRARY_DIRS})
    target_link_libraries(zlib INTERFACE ${zlib_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS zlib)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES zlib)
        endif()
    endif()
    
else()
    message(STATUS "zlib Not Found")
    
    set(zlib_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/zlib-install)

    fetch_only(zlib
        https://github.com/madler/zlib.git
        v1.3.1
        configure
    )
    message(STATUS "Building zlib...")
    include(ProcessorCount)
    ProcessorCount(N)
    if(N EQUAL 0)
        set(N 1)
    endif()
    if(NOT EXISTS ${zlib_SOURCE_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${zlib_SOURCE_DIR}/Makefile)
        execute_process(
            COMMAND ${ENV_COMMAND}
                "CC=${ORIGINAL_ENV_CC}" "CFLAGS=${MAGICKPP_FLAGS}"
                "MAKE=${MAKE_COMMAND}"
                ${SH_COMMAND} configure
                --static --prefix=${zlib_PREFIX}
            WORKING_DIRECTORY ${zlib_SOURCE_DIR}
        )
    endif()
    execute_process(
        COMMAND ${MAKE_COMMAND} -j${N} install
        WORKING_DIRECTORY ${zlib_SOURCE_DIR}
    )

    include(cmake/linker.cmake)
    add_prefix(${zlib_PREFIX})
    pkg_check_modules(zlib QUIET zlib)
    if(NOT zlib_FOUND)
        message(FATAL_ERROR "Failed to build zlib")
    endif()
    add_library(zlib INTERFACE)
    target_static_link(zlib
        LIBRARY_DIRS ${zlib_STATIC_LIBRARY_DIRS}
        LIBRARIES ${zlib_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS zlib)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            install_prefix_libs(${zlib_PREFIX}
                LIBRARY_DIRS ${zlib_STATIC_LIBRARY_DIRS}
                LIBRARIES ${zlib_STATIC_LIBRARIES}
            )
        endif()
    endif()
endif()

include(cmake/fetch.cmake)
option(WEBCFACE_FIND_ZLIB "try find_package(zlib) (while building zlib)" ${WEBCFACE_FIND_LIBS})

# target = webcface-zlib-linker
unset(zlib_FOUND CACHE)
if(WEBCFACE_FIND_ZLIB)
    pkg_check_modules(zlib QUIET zlib)
endif()
if(zlib_FOUND)
    list(APPEND WEBCFACE_SUMMARY "zlib: ${zlib_VERSION} found at ${zlib_PREFIX}")
    add_library(webcface-zlib-linker INTERFACE)
    target_link_directories(webcface-zlib-linker INTERFACE ${zlib_LIBRARY_DIRS})
    target_link_libraries(webcface-zlib-linker INTERFACE ${zlib_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-zlib-linker)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES zlib)
        endif()
    endif()
    
else()
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
    if(NOT EXISTS ${zlib_BINARY_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${zlib_BINARY_DIR}/Makefile)
        if(EXISTS ${zlib_BINARY_DIR}/Makefile)
            execute_process(
                COMMAND ${MAKE_COMMAND} clean
                WORKING_DIRECTORY ${zlib_BINARY_DIR}
            )
        endif()
        file(REMOVE_RECURSE ${zlib_PREFIX})
        include(cmake/flags.cmake)
        init_flags()
        execute_process(
            COMMAND ${ENV_COMMAND}
                "CC=${ORIGINAL_ENV_CC}"
                "CFLAGS=${WEBCFACE_FLAGS}" "LDFLAGS=${WEBCFACE_LDFLAGS}"
                "MAKE=${MAKE_COMMAND}"
                ${SH_COMMAND} ${zlib_SOURCE_DIR}/configure
                --static --prefix=${zlib_PREFIX}
            WORKING_DIRECTORY ${zlib_BINARY_DIR}
        )
    endif()
    if(NOT EXISTS ${zlib_BINARY_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${zlib_BINARY_DIR}/Makefile)
        message(FATAL_ERROR "Failed to configure zlib")
    endif()
    execute_process(
        COMMAND ${MAKE_COMMAND} -j${N} install
        WORKING_DIRECTORY ${zlib_BINARY_DIR}
    )

    include(cmake/linker.cmake)
    add_prefix(${zlib_PREFIX})
    pkg_check_modules(zlib QUIET zlib)
    if(NOT zlib_FOUND)
        message(FATAL_ERROR "Failed to build zlib")
    endif()
    add_library(webcface-zlib-linker INTERFACE)
    target_static_link(webcface-zlib-linker
        BUILD_LIBRARY_DIRS ${zlib_STATIC_LIBRARY_DIRS}
        DEBUG_LIBRARIES ${zlib_STATIC_LIBRARIES}
        RELEASE_LIBRARIES ${zlib_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-zlib-linker)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            install_prefix_libs(${zlib_PREFIX}
                LIBRARY_DIRS ${zlib_STATIC_LIBRARY_DIRS}
                LIBRARIES ${zlib_STATIC_LIBRARIES}
            )
        endif()
    endif()
endif()

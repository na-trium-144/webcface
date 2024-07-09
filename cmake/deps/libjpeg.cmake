include(cmake/fetch.cmake)
option(WEBCFACE_FIND_JPEG "try pkg_check_modules(libjpeg) (while building Magick++)" ${WEBCFACE_FIND_LIBS})

# target = libjpeg
unset(libjpeg_FOUND CACHE)
if(WEBCFACE_FIND_JPEG)
    pkg_check_modules(libjpeg QUIET libjpeg)
endif()
if(libjpeg_FOUND)
    list(APPEND WEBCFACE_SUMMARY "libjpeg: ${libjpeg_VERSION} found at ${libjpeg_PREFIX}")
    add_library(libjpeg INTERFACE)
    target_link_directories(libjpeg INTERFACE ${libjpeg_LIBRARY_DIRS})
    target_link_libraries(libjpeg INTERFACE ${libjpeg_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS libjpeg)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES libjpeg)
        endif()
    endif()

else()
    set(libjpeg_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/libjpeg-install)
    fetch_only(libjpeg-turbo
        https://github.com/libjpeg-turbo/libjpeg-turbo.git
        3.0.3
        CMakeLists.txt
    )
    message(STATUS "Building libjpeg-turbo...")
    include(ProcessorCount)
    ProcessorCount(N)
    if(N EQUAL 0)
        set(N 1)
    endif()
    include(cmake/flags.cmake)
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${libjpeg-turbo_SOURCE_DIR} -B${libjpeg-turbo_BINARY_DIR}
            -DCMAKE_INSTALL_PREFIX=${libjpeg_PREFIX}
            -DENABLE_SHARED=OFF -DWITH_TURBOJPEG=OFF
            ${WEBCFACE_CMAKE_PROPS}
    )
    execute_process(COMMAND ${CMAKE_COMMAND} --build ${libjpeg-turbo_BINARY_DIR} -t install -j${N})

    include(cmake/linker.cmake)
    add_prefix(${libjpeg_PREFIX})
    pkg_check_modules(libjpeg QUIET libjpeg)
    if(NOT libjpeg_FOUND)
        message(FATAL_ERROR "Failed to build libjpeg")
    endif()
    add_library(libjpeg INTERFACE)
    target_static_link(libjpeg
        BUILD_LIBRARY_DIRS ${libjpeg_STATIC_LIBRARY_DIRS}
        DEBUG_LIBRARIES ${libjpeg_STATIC_LIBRARIES}
        RELEASE_LIBRARIES ${libjpeg_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS libjpeg)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            install_prefix_libs(${libjpeg_PREFIX}
                LIBRARY_DIRS ${libjpeg_STATIC_LIBRARY_DIRS}
                LIBRARIES ${libjpeg_STATIC_LIBRARIES}
            )
        endif()
    endif()
endif()


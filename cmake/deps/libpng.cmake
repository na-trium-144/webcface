include(cmake/fetch.cmake)
option(WEBCFACE_FIND_PNG "try pkg_check_modules(libpng) (while building Magick++)" ${WEBCFACE_FIND_LIBS})

# target = libpng
unset(libpng_FOUND CACHE)
if(WEBCFACE_FIND_PNG)
    pkg_check_modules(libpng QUIET libpng)
endif()
if(libpng_FOUND)
    list(APPEND WEBCFACE_SUMMARY "libpng: ${libpng_VERSION} found at ${libpng_PREFIX}")
    add_library(libpng INTERFACE)
    target_link_directories(libpng INTERFACE ${libpng_LIBRARY_DIRS})
    target_link_libraries(libpng INTERFACE ${libpng_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS libpng)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES libpng)
        endif()
    endif()

else()
    include(cmake/deps/zlib.cmake)

    set(libpng_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/libpng-install)
    fetch_only(libpng
        https://github.com/pnggroup/libpng.git
        v1.6.43
        CMakeLists.txt
    )
    message(STATUS "Building libpng...")
    include(ProcessorCount)
    ProcessorCount(N)
    if(N EQUAL 0)
        set(N 1)
    endif()
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${libpng_SOURCE_DIR} -B${libpng_BINARY_DIR}
            -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${libpng_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DPNG_SHARED=OFF -DPNG_FRAMEWORK=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF
            -DCMAKE_POSITION_INDEPENDENT_CODE=${WEBCFACE_PIC}
    )
    execute_process(COMMAND ${CMAKE_COMMAND} --build ${libpng_BINARY_DIR} -t install -j${N})

    include(cmake/linker.cmake)
    add_prefix(${libpng_PREFIX})
    pkg_check_modules(libpng QUIET libpng)
    if(NOT libpng_FOUND)
        message(FATAL_ERROR "Failed to build libpng")
    endif()
    add_library(libpng INTERFACE)
    target_link_libraries(libpng INTERFACE zlib)
    target_static_link(libpng
        LIBRARY_DIRS ${libpng_STATIC_LIBRARY_DIRS}
        LIBRARIES ${libpng_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS libpng)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            install_prefix_libs(${libpng_PREFIX}
                LIBRARY_DIRS ${libpng_STATIC_LIBRARY_DIRS}
                LIBRARIES ${libpng_STATIC_LIBRARIES}
            )
        endif()
    endif()
endif()

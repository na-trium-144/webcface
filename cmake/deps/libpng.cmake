include(cmake/fetch.cmake)
option(WEBCFACE_FIND_PNG "try pkg_check_modules(libpng) (while building Magick++)" ${WEBCFACE_FIND_LIBS})

# target = webcface-libpng-linker
unset(libpng_FOUND CACHE)
if(WEBCFACE_FIND_PNG)
    pkg_check_modules(libpng QUIET libpng)
endif()
if(libpng_FOUND)
    list(APPEND WEBCFACE_SUMMARY "libpng: ${libpng_VERSION} found at ${libpng_PREFIX}")
    add_library(webcface-libpng-linker INTERFACE)
    target_link_directories(webcface-libpng-linker INTERFACE ${libpng_LIBRARY_DIRS})
    target_link_libraries(webcface-libpng-linker INTERFACE ${libpng_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libpng-linker)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES libpng)
        endif()
    endif()

else()
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
    include(cmake/flags.cmake)
    if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES STREQUAL "")
        foreach(arch IN LISTS CMAKE_OSX_ARCHITECTURES)
            # universalビルド非対応
            init_flags(ARCH ${arch})
            execute_process(
                COMMAND ${CMAKE_COMMAND} ${libpng_SOURCE_DIR} -B${libpng_BINARY_DIR}_${arch}
                    -DCMAKE_INSTALL_PREFIX=${libpng_PREFIX}
                    -DPNG_SHARED=OFF -DPNG_FRAMEWORK=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF
                    "-DZLIB_ROOT=${zlib_PREFIX}"
                    ${WEBCFACE_CMAKE_PROPS}
            )
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${libpng_BINARY_DIR}_${arch} -t install -j${N})
            file(MAKE_DIRECTORY ${libpng_PREFIX}_${arch})
            file(COPY ${libpng_PREFIX}/include DESTINATION ${libpng_PREFIX}_${arch})
            file(COPY ${libpng_PREFIX}/lib DESTINATION ${libpng_PREFIX}_${arch})
            list(APPEND libpng_A_DIRS ${libpng_PREFIX}_${arch}/lib)
            file(GLOB libpng_A_FILES
                RELATIVE ${libpng_PREFIX}_${arch}/lib
                ${libpng_PREFIX}_${arch}/lib/*.a
            )
        endforeach()
        lipo(
            DIRECTORIES ${libpng_A_DIRS}
            FILES ${libpng_A_FILES}
            DESTINATION ${libpng_PREFIX}/lib
        )
    else()
        init_flags()
        execute_process(
            COMMAND ${CMAKE_COMMAND} ${libpng_SOURCE_DIR} -B${libpng_BINARY_DIR}
                -DCMAKE_INSTALL_PREFIX=${libpng_PREFIX}
                -DPNG_SHARED=OFF -DPNG_FRAMEWORK=OFF -DPNG_TESTS=OFF -DPNG_TOOLS=OFF
                "-DZLIB_ROOT=${zlib_PREFIX}"
                ${WEBCFACE_CMAKE_PROPS}
        )
        execute_process(COMMAND ${CMAKE_COMMAND} --build ${libpng_BINARY_DIR} -t install -j${N})
    endif()

    include(cmake/linker.cmake)
    add_prefix(${libpng_PREFIX})
    pkg_check_modules(libpng QUIET libpng)
    if(NOT libpng_FOUND)
        message(FATAL_ERROR "Failed to build libpng")
    endif()
    add_library(webcface-libpng-linker INTERFACE)
    target_link_libraries(webcface-libpng-linker INTERFACE webcface-zlib-linker)
    target_static_link(webcface-libpng-linker
        BUILD_LIBRARY_DIRS ${libpng_STATIC_LIBRARY_DIRS}
        DEBUG_LIBRARIES ${libpng_STATIC_LIBRARIES}
        RELEASE_LIBRARIES ${libpng_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libpng-linker)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            install_prefix_libs(${libpng_PREFIX}
                LIBRARY_DIRS ${libpng_STATIC_LIBRARY_DIRS}
                LIBRARIES ${libpng_STATIC_LIBRARIES}
            )
        endif()
    endif()
endif()

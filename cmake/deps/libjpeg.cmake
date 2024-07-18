include(cmake/fetch.cmake)
option(WEBCFACE_FIND_JPEG "try pkg_check_modules(libjpeg) (while building Magick++)" ${WEBCFACE_FIND_LIBS})

# target = webcface-libjpeg-linker
unset(libjpeg_FOUND CACHE)
if(WEBCFACE_FIND_JPEG)
    pkg_check_modules(libjpeg QUIET libjpeg)
endif()
if(libjpeg_FOUND)
    list(APPEND WEBCFACE_SUMMARY "libjpeg: ${libjpeg_VERSION} found at ${libjpeg_PREFIX}")
    add_library(webcface-libjpeg-linker INTERFACE)
    target_link_directories(webcface-libjpeg-linker INTERFACE ${libjpeg_LIBRARY_DIRS})
    target_link_libraries(webcface-libjpeg-linker INTERFACE ${libjpeg_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libjpeg-linker)
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
    if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES STREQUAL "")
        foreach(arch IN LISTS CMAKE_OSX_ARCHITECTURES)
            # universalビルド非対応
            init_flags(ARCH ${arch})
            execute_process(
                COMMAND ${CMAKE_COMMAND} ${libjpeg-turbo_SOURCE_DIR} -B${libjpeg-turbo_BINARY_DIR}_${arch}
                    -DCMAKE_INSTALL_PREFIX=${libjpeg_PREFIX}
                    -DENABLE_SHARED=OFF -DWITH_TURBOJPEG=OFF
                    ${WEBCFACE_CMAKE_PROPS}
            )
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${libjpeg-turbo_BINARY_DIR}_${arch} -t install -j${N})
            file(MAKE_DIRECTORY ${libjpeg_PREFIX}_${arch})
            file(COPY ${libjpeg_PREFIX}/include DESTINATION ${libjpeg_PREFIX}_${arch})
            file(COPY ${libjpeg_PREFIX}/lib DESTINATION ${libjpeg_PREFIX}_${arch})
            list(APPEND libjpeg_A_DIRS ${libjpeg_PREFIX}_${arch}/lib)
            file(GLOB libjpeg_A_FILES
                RELATIVE ${libjpeg_PREFIX}_${arch}/lib
                ${libjpeg_PREFIX}_${arch}/lib/*.a
            )
        endforeach()
        lipo(
            DIRECTORIES ${libjpeg_A_DIRS}
            FILES ${libjpeg_A_FILES}
            DESTINATION ${libjpeg_PREFIX}/lib
        )
    else()
        init_flags()
        execute_process(
            COMMAND ${CMAKE_COMMAND} ${libjpeg-turbo_SOURCE_DIR} -B${libjpeg-turbo_BINARY_DIR}
                -DCMAKE_INSTALL_PREFIX=${libjpeg_PREFIX}
                -DENABLE_SHARED=OFF -DWITH_TURBOJPEG=OFF
                ${WEBCFACE_CMAKE_PROPS}
        )
        execute_process(COMMAND ${CMAKE_COMMAND} --build ${libjpeg-turbo_BINARY_DIR} -t install -j${N})
    endif()
    include(cmake/linker.cmake)
    add_prefix(${libjpeg_PREFIX})
    pkg_check_modules(libjpeg QUIET libjpeg)
    if(NOT libjpeg_FOUND)
        message(FATAL_ERROR "Failed to build libjpeg")
    endif()
    add_library(webcface-libjpeg-linker INTERFACE)
    target_static_link(webcface-libjpeg-linker
        BUILD_LIBRARY_DIRS ${libjpeg_STATIC_LIBRARY_DIRS}
        DEBUG_LIBRARIES ${libjpeg_STATIC_LIBRARIES}
        RELEASE_LIBRARIES ${libjpeg_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libjpeg-linker)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            install_prefix_libs(${libjpeg_PREFIX}
                LIBRARY_DIRS ${libjpeg_STATIC_LIBRARY_DIRS}
                LIBRARIES ${libjpeg_STATIC_LIBRARIES}
            )
        endif()
    endif()
endif()


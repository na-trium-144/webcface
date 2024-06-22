include(cmake/fetch.cmake)
option(WEBCFACE_FIND_WEBP "try pkg_check_modules(libwebp) (while building Magick++)" ${WEBCFACE_FIND_LIBS})

# target = libwebp
unset(libwebp_FOUND CACHE)
if(WEBCFACE_FIND_WEBP)
    pkg_check_modules(libwebp QUIET libwebp)
endif()
if(libwebp_FOUND)
    message(STATUS "libwebp ${libwebp_VERSION} Found: ${libwebp_PREFIX}")
    add_library(libwebp INTERFACE)
    target_link_directories(libwebp INTERFACE ${libwebp_LIBRARY_DIRS})
    target_link_libraries(libwebp INTERFACE ${libwebp_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS libwebp)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES libwebp)
        endif()
    endif()

else()
    message(STATUS "libwebp Not Found")
    set(libwebp_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/libwebp-install)
    fetch_only(libwebp
        https://github.com/webmproject/libwebp.git
        v1.4.0
        CMakeLists.txt
    )
    message(STATUS "Building libwebp...")
    include(ProcessorCount)
    ProcessorCount(N)
    if(N EQUAL 0)
        set(N 1)
    endif()
    execute_process(
        COMMAND ${CMAKE_COMMAND} ${libwebp_SOURCE_DIR} -B${libwebp_BINARY_DIR}
            -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${libwebp_PREFIX}
            -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
            -DBUILD_SHARED_LIBS=OFF -DWEBP_LINK_STATIC=ON
            -DWEBP_BUILD_ANIM_UTILS=OFF -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF
            -DWEBP_BUILD_GIF2WEBP=OFF -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF
            -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF
            -DCMAKE_POSITION_INDEPENDENT_CODE=${WEBCFACE_PIC}
    )
    execute_process(COMMAND ${CMAKE_COMMAND} --build ${libwebp_BINARY_DIR} -t install -j${N})

    include(cmake/linker.cmake)
    add_prefix(${libwebp_PREFIX})
    pkg_check_modules(libwebp QUIET libwebp)
    if(NOT libwebp_FOUND)
        message(FATAL_ERROR "Failed to build libwebp")
    endif()
    add_library(libwebp INTERFACE)
    target_static_link(libwebp
        LIBRARY_DIRS ${libwebp_STATIC_LIBRARY_DIRS}
        LIBRARIES ${libwebp_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS libwebp)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            install_prefix_libs(${libwebp_PREFIX}
                LIBRARY_DIRS ${libwebp_STATIC_LIBRARY_DIRS}
                LIBRARIES ${libwebp_STATIC_LIBRARIES}
            )
        endif()
    endif()
endif()

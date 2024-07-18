include(cmake/fetch.cmake)
option(WEBCFACE_FIND_WEBP "try pkg_check_modules(libwebp) (while building Magick++)" ${WEBCFACE_FIND_LIBS})

# target = webcface-libwebp-linker
unset(libwebp_FOUND CACHE)
if(WEBCFACE_FIND_WEBP)
    pkg_check_modules(libwebp QUIET libwebp)
endif()
if(libwebp_FOUND)
    list(APPEND WEBCFACE_SUMMARY "libwebp: ${libwebp_VERSION} found at ${libwebp_PREFIX}")
    add_library(webcface-libwebp-linker INTERFACE)
    target_link_directories(webcface-libwebp-linker INTERFACE ${libwebp_LIBRARY_DIRS})
    target_link_libraries(webcface-libwebp-linker INTERFACE ${libwebp_LIBRARIES})

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libwebp-linker)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES libwebp)
        endif()
    endif()

else()
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
    include(cmake/flags.cmake)
    if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES STREQUAL "")
        foreach(arch IN LISTS CMAKE_OSX_ARCHITECTURES)
            # universalビルド非対応
            init_flags(ARCH ${arch})
            execute_process(
                COMMAND ${CMAKE_COMMAND} ${libwebp_SOURCE_DIR} -B${libwebp_BINARY_DIR}_${arch}
                    -DCMAKE_INSTALL_PREFIX=${libwebp_PREFIX}
                    -DBUILD_SHARED_LIBS=OFF -DWEBP_LINK_STATIC=ON
                    -DWEBP_BUILD_ANIM_UTILS=OFF -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF
                    -DWEBP_BUILD_GIF2WEBP=OFF -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF
                    -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF
                    ${WEBCFACE_CMAKE_PROPS}
            )
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${libwebp_BINARY_DIR}_${arch} -t install -j${N})
            file(MAKE_DIRECTORY ${libwebp_PREFIX}_${arch})
            file(COPY ${libwebp_PREFIX}/include DESTINATION ${libwebp_PREFIX}_${arch})
            file(COPY ${libwebp_PREFIX}/lib DESTINATION ${libwebp_PREFIX}_${arch})
            list(APPEND libwebp_A_DIRS ${libwebp_PREFIX}_${arch}/lib)
            file(GLOB libwebp_A_FILES
                RELATIVE ${libwebp_PREFIX}_${arch}/lib
                ${libwebp_PREFIX}_${arch}/lib/*.a
            )
        endforeach()
        lipo(
            DIRECTORIES ${libwebp_A_DIRS}
            FILES ${libwebp_A_FILES}
            DESTINATION ${libwebp_PREFIX}/lib
        )
    else()
        init_flags()
        execute_process(
            COMMAND ${CMAKE_COMMAND} ${libwebp_SOURCE_DIR} -B${libwebp_BINARY_DIR}
                -DCMAKE_INSTALL_PREFIX=${libwebp_PREFIX}
                -DBUILD_SHARED_LIBS=OFF -DWEBP_LINK_STATIC=ON
                -DWEBP_BUILD_ANIM_UTILS=OFF -DWEBP_BUILD_CWEBP=OFF -DWEBP_BUILD_DWEBP=OFF
                -DWEBP_BUILD_GIF2WEBP=OFF -DWEBP_BUILD_IMG2WEBP=OFF -DWEBP_BUILD_VWEBP=OFF
                -DWEBP_BUILD_WEBPINFO=OFF -DWEBP_BUILD_WEBPMUX=OFF -DWEBP_BUILD_EXTRAS=OFF
                ${WEBCFACE_CMAKE_PROPS}
        )
        execute_process(COMMAND ${CMAKE_COMMAND} --build ${libwebp_BINARY_DIR} -t install -j${N})
    endif()
    include(cmake/linker.cmake)
    add_prefix(${libwebp_PREFIX})
    set(libwebp_packages libsharpyuv libwebpmux libwebpdemux libwebp)
    foreach(libwebp_each IN LISTS libwebp_packages)
        pkg_check_modules(${libwebp_each} QUIET ${libwebp_each})
        if(NOT ${libwebp_each}_FOUND)
            message(FATAL_ERROR "Failed to build ${libwebp_each}")
        endif()
    endforeach()
    add_library(webcface-libwebp-linker INTERFACE)
    target_static_link(webcface-libwebp-linker
        BUILD_LIBRARY_DIRS ${libwebp_STATIC_LIBRARY_DIRS}
        # ここでlibwebpmuxなども渡すとなぜかリンクエラーになってしまう
        DEBUG_LIBRARIES ${libwebp_STATIC_LIBRARIES}
        RELEASE_LIBRARIES ${libwebp_STATIC_LIBRARIES}
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS webcface-libwebp-linker)
        if(NOT WEBCFACE_SHARED)
            include(cmake/linker.cmake)
            foreach(libwebp_each IN LISTS libwebp_packages)
                install_prefix_libs(${libwebp_PREFIX}
                    LIBRARY_DIRS ${${libwebp_each}_STATIC_LIBRARY_DIRS}
                    LIBRARIES ${${libwebp_each}_STATIC_LIBRARIES}
                )
            endforeach()
        endif()
    endif()
endif()

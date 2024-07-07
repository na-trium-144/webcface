macro(target_static_link LINKER_TARGET)
    # staticライブラリをリンクする際、MacOSでは--exclude-libsが使えず、その代わり-hidden-lでライブラリを渡す必要がある
    cmake_parse_arguments(LINKER
        ""
        ""
        "BUILD_LIBRARY_DIRS;INSTALL_LIBRARY_DIRS;DEBUG_LIBRARIES;RELEASE_LIBRARIES"
        ${ARGN}
    )
    if(NOT "${LINKER_UNPARSED_ARGUMENTS}" STREQUAL "")
        message(FATAL_ERROR "Invalid argument for target_static_link(${LINKER_TARGET}): ${LINKER_UNPARSED_ARGUMENTS}")
    endif()
    foreach(dir IN LISTS LINKER_BUILD_LIBRARY_DIRS)
        target_link_directories(${LINKER_TARGET} INTERFACE $<BUILD_INTERFACE:${dir}>)
    endforeach()
    foreach(dir IN LISTS LINKER_INSTALL_LIBRARY_DIRS)
        target_link_directories(${LINKER_TARGET} INTERFACE $<INSTALL_INTERFACE:${dir}>)
    endforeach()
    # msvcはdebugかreleaseかは常に一致させるので両方インストールしていい
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
        foreach(lib IN LISTS LINKER_DEBUG_LIBRARIES)
            if(WEBCFACE_SHARED AND APPLE)
                target_link_libraries(${LINKER_TARGET} INTERFACE debug -Wl,-hidden-l${lib})
            else()
                target_link_libraries(${LINKER_TARGET} INTERFACE debug ${lib})
            endif()
        endforeach()
    endif()
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        foreach(lib IN LISTS LINKER_RELEASE_LIBRARIES)
            if(WEBCFACE_SHARED AND APPLE)
                target_link_libraries(${LINKER_TARGET} INTERFACE optimized -Wl,-hidden-l${lib})
            else()
                target_link_libraries(${LINKER_TARGET} INTERFACE optimized ${lib})
            endif()
        endforeach()
    endif()
endmacro()

macro(add_prefix PREFIX)
    list(APPEND CMAKE_PREFIX_PATH ${PREFIX})
    if(MINGW)
        set(ENV{PKG_CONFIG_PATH} "${PREFIX}/lib/pkgconfig;$ENV{PKG_CONFIG_PATH}")
    else()
        set(ENV{PKG_CONFIG_PATH} "${PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    endif()
endmacro()

macro(install_prefix_libs LINKER_PREFIX)
    cmake_parse_arguments(LINKER
        ""
        ""
        "LIBRARY_DIRS;LIBRARIES"
        ${ARGN}
    )
    if(NOT "${LINKER_UNPARSED_ARGUMENTS}" STREQUAL "" OR NOT "${LINKER_KEYWORDS_MISSING_VALUES}" STREQUAL "")
        message(FATAL_ERROR "Invalid argument for install_prefix_libs(): ${LINKER_UNPARSED_ARGUMENTS} ${LINKER_KEYWORDS_MISSING_VALUES}")
    endif()
    set(linker_PKGCONFIG_LIBS )
    foreach(dir IN LISTS LINKER_LIBRARY_DIRS)
        if(NOT dir MATCHES "^${CMAKE_CURRENT_BINARY_DIR}/")
            list(APPEND linker_PKGCONFIG_LIBS -L${dir})
        endif()
    endforeach()
    foreach(lib IN LISTS LINKER_LIBRARIES)
        install(FILES ${LINKER_PREFIX}/lib/lib${lib}.a
            DESTINATION lib
            OPTIONAL
        )
        list(APPEND linker_PKGCONFIG_LIBS -l${lib})
    endforeach()
    list(INSERT WEBCFACE_PKGCONFIG_LIBS 0 ${linker_PKGCONFIG_LIBS})
endmacro()

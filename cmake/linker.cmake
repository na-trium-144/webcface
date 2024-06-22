macro(target_static_link LINKER_TARGET)
    # staticライブラリをリンクする際、MacOSでは--exclude-libsが使えず、その代わり-hidden-lでライブラリを渡す必要がある
    cmake_parse_arguments(LINKER
        ""
        ""
        "LIBRARY_DIRS;LIBRARIES"
        ${ARGN}
    )
    if(NOT "${LINKER_UNPARSED_ARGUMENTS}" STREQUAL "" OR NOT "${LINKER_KEYWORDS_MISSING_VALUES}" STREQUAL "")
        message(FATAL_ERROR "Invalid argument for target_static_link(${LINKER_TARGET}): ${LINKER_UNPARSED_ARGUMENTS} ${LINKER_KEYWORDS_MISSING_VALUES}")
    endif()
    target_link_directories(${LINKER_TARGET} INTERFACE
        $<BUILD_INTERFACE:${LINKER_LIBRARY_DIRS}>
    )
    if(WEBCFACE_SHARED AND APPLE)
        foreach(lib IN LISTS LINKER_LIBRARIES)
            target_link_libraries(${LINKER_TARGET} INTERFACE -Wl,-hidden-l${lib})
        endforeach()
    else()
        target_link_libraries(${LINKER_TARGET} INTERFACE ${LINKER_LIBRARIES})
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
    foreach(dir IN LISTS LINKER_LIBRARY_DIRS)
        if(NOT dir MATCHES "^${CMAKE_CURRENT_BINARY_DIR}/")
            list(APPEND WEBCFACE_PKGCONFIG_LIBS -L${dir})
        endif()
    endforeach()
    foreach(lib IN LISTS LINKER_LIBRARIES)
        install(FILES ${LINKER_PREFIX}/lib/lib${lib}.a
            DESTINATION lib
            OPTIONAL
        )
        list(APPEND WEBCFACE_PKGCONFIG_LIBS -l${lib})
    endforeach()
endmacro()

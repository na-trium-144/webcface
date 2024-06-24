include(cmake/fetch.cmake)
option(WEBCFACE_FIND_MAGICK "try pkg_check_modules(Magick++)" ${WEBCFACE_FIND_LIBS})

# target = magickpp-linker

include(FindPkgConfig)
# あとでbuildしたmagickppをprefixに追加して再度pkg_check_modulesしているため、
# ここでは毎回cacheをクリアする必要ある
unset(Magickpp_FOUND CACHE)
if(WEBCFACE_FIND_MAGICK)
    pkg_check_modules(Magickpp QUIET Magick++)
endif()
if(Magickpp_FOUND)
    set(Magickpp_INSTALLED 1)
    list(APPEND WEBCFACE_SUMMARY "imagemagick: ${Magickpp_VERSION} found at ${Magickpp_PREFIX}")
    add_library(magickpp-linker INTERFACE)
    target_include_directories(magickpp-linker INTERFACE ${Magickpp_INCLUDE_DIRS})
    target_compile_options(magickpp-linker INTERFACE ${Magickpp_CFLAGS_OTHER})
    target_link_directories(magickpp-linker INTERFACE ${Magickpp_LIBRARY_DIRS})
    target_link_libraries(magickpp-linker INTERFACE ${Magickpp_LIBRARIES})
    if(Magickpp_VERSION MATCHES "^7\.")
        target_compile_definitions(magickpp-linker INTERFACE WEBCFACE_MAGICK_VER7)
    endif()

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS magickpp-linker)
        if(NOT WEBCFACE_SHARED)
            list(APPEND WEBCFACE_PKGCONFIG_REQUIRES Magickpp)
        endif()
    endif()

endif()

if(WEBCFACE_FIND_MAGICK AND NOT Magickpp_FOUND AND CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    find_path(MAGICKPP_WIN_INCLUDE_DIR Magick++.h PATH_SUFFIXES include)
    if(NOT MAGICKPP_WIN_INCLUDE_DIR STREQUAL "MAGICKPP_WIN_INCLUDE_DIR-NOTFOUND")
        set(Magickpp_FOUND 1)
        set(Magickpp_INSTALLED 1)
        get_filename_component(MAGICKPP_PREFIX ${MAGICKPP_WIN_INCLUDE_DIR} DIRECTORY)
        list(APPEND WEBCFACE_SUMMARY "imagemagick: found at ${MAGICKPP_PREFIX}")
        set(MAGICKPP_LIB_DIR "${MAGICKPP_PREFIX}/lib")
        if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
            file(GLOB MAGICKPP_RL_LIBS
                RELATIVE ${MAGICKPP_LIB_DIR}
                ${MAGICKPP_LIB_DIR}/CORE_RL_*.lib
            )
            if(MAGICKPP_RL_LIBS STREQUAL "" AND NOT WEBCFACE_CONFIG_ALL)
                message(FATAL_ERROR "Release build of ImageMagick Not Found.")
            endif()
            foreach(lib IN LISTS MAGICKPP_RL_LIBS)
                list(APPEND MAGICKPP_LIBS optimized ${MAGICKPP_LIB_DIR}/${lib})
            endforeach()
        endif()
        if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
            file(GLOB MAGICKPP_DB_LIBS
                RELATIVE ${MAGICKPP_LIB_DIR}
                ${MAGICKPP_LIB_DIR}/CORE_DB_*.lib
            )
            if(MAGICKPP_DB_LIBS STREQUAL "" AND NOT WEBCFACE_CONFIG_ALL)
                message(FATAL_ERROR "Debug build of ImageMagick Not Found.")
            endif()
            foreach(lib IN LISTS MAGICKPP_DB_LIBS)
                list(APPEND MAGICKPP_LIBS debug ${MAGICKPP_LIB_DIR}/${lib})
            endforeach()
        endif()
        add_library(magickpp-linker INTERFACE)
        target_include_directories(magickpp-linker INTERFACE ${MAGICKPP_WIN_INCLUDE_DIR})
        target_link_libraries(magickpp-linker INTERFACE ${MAGICKPP_LIBS})
        target_compile_definitions(magickpp-linker INTERFACE WEBCFACE_MAGICK_VER7)

        if(WEBCFACE_INSTALL)
            list(APPEND WEBCFACE_EXPORTS magickpp-linker)
            if(NOT WEBCFACE_SHARED)
                list(APPEND WEBCFACE_PKGCONFIG_LIBS -L${MAGICKPP_LIB_DIR})
                foreach(lib IN LISTS MAGICKPP_DB_LIBS MAGICKPP_RL_LIBS)
                    list(APPEND WEBCFACE_PKGCONFIG_LIBS -l${lib})
                endforeach()
            endif()
        endif()

    endif()
endif()

if(NOT Magickpp_FOUND)
    set(Magickpp_INSTALLED 0)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        include(cmake/deps/magickpp-build-msvc.cmake)
    else()
        include(cmake/deps/magickpp-build-unix.cmake)
    endif()
endif()


set(MAGICKPP_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/_deps/imagemagick-install)

find_program(SH_COMMAND sh)
find_program(ENV_COMMAND env)
find_program(MAKE_COMMAND make)
find_program(CHMOD_COMMAND chmod)
find_program(CYGPATH_COMMAND cygpath)
if(WEBCFACE_SYSTEM_PATH_WINDOWS AND NOT CYGPATH_COMMAND STREQUAL "CYGPATH_COMMAND-NOTFOUND")
    execute_process(
        COMMAND ${CYGPATH_COMMAND} -u "${MAGICKPP_PREFIX}"
        OUTPUT_VARIABLE MAGICKPP_PREFIX_UNIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(MAGICKPP_PREFIX_UNIX "${MAGICKPP_PREFIX}")
endif()

include(cmake/deps/libjpeg.cmake)
include(cmake/deps/zlib.cmake)
include(cmake/deps/libpng.cmake)
include(cmake/deps/libwebp.cmake)

fetch_only(imagemagick
    https://github.com/ImageMagick/ImageMagick.git
    841f033f0
    configure
)
if(WEBCFACE_SYSTEM_PATH_WINDOWS AND NOT CYGPATH_COMMAND STREQUAL "CYGPATH_COMMAND-NOTFOUND")
    execute_process(
        COMMAND ${CYGPATH_COMMAND} -u "${imagemagick_SOURCE_DIR}"
        OUTPUT_VARIABLE imagemagick_SOURCE_DIR_UNIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else()
    set(imagemagick_SOURCE_DIR_UNIX "${imagemagick_SOURCE_DIR}")
endif()

message(STATUS "Building Magick++...")
if(NOT EXISTS ${imagemagick_BINARY_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick_BINARY_DIR}/Makefile)
    execute_process(
        COMMAND ${CHMOD_COMMAND} +x winpath.sh # バグ?
        WORKING_DIRECTORY ${imagemagick_SOURCE_DIR}
    )
    if(EXISTS ${imagemagick_BINARY_DIR}/Makefile)
        execute_process(
            COMMAND ${MAKE_COMMAND} clean
            WORKING_DIRECTORY ${imagemagick_BINARY_DIR}
        )
    endif()
    file(REMOVE_RECURSE ${MAGICKPP_PREFIX})
    include(cmake/flags.cmake)
    init_flags()
    execute_process(
        # mingwでは ./configure はつかえない
        COMMAND ${SH_COMMAND} "${imagemagick_SOURCE_DIR_UNIX}/configure"
            "CC=${ORIGINAL_ENV_CC}" "CXX=${ORIGINAL_ENV_CXX}"
            "CFLAGS=${WEBCFACE_FLAGS}" "CXXFLAGS=${WEBCFACE_FLAGS}" "LDFLAGS=${WEBCFACE_LDFLAGS}"
            "MAKE=${MAKE_COMMAND}"
            --prefix=${MAGICKPP_PREFIX_UNIX} --disable-shared --enable-static
            --disable-openmp # <- for multi threading
            --without-utilities --disable-hdri --with-quantum-depth=8
            --without-modules --without-perl --without-bzlib --without-djvu --without-dps
            --without-fontconfig --without-freetype --without-gvc --without-heic
            --without-jbig --without-jxl --without-dmr --without-lqr --without-lcms
            --without-lzma --without-openexr --without-openjp2 --without-pango
            --without-raqm --without-raw --without-tiff --without-wmf
            --without-xml --without-zlib --without-zstd --without-x --without-zip
        WORKING_DIRECTORY ${imagemagick_BINARY_DIR}
    )
endif()
if(NOT EXISTS ${imagemagick_BINARY_DIR}/Makefile OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick_BINARY_DIR}/Makefile)
    message(FATAL_ERROR "Failed to configure ImageMagick")
endif()
include(ProcessorCount)
ProcessorCount(N)
if(N EQUAL 0)
    set(N 1)
endif()
execute_process(
    COMMAND ${MAKE_COMMAND} -j${N} install
    WORKING_DIRECTORY ${imagemagick_BINARY_DIR}
)

include(cmake/linker.cmake)
add_prefix(${MAGICKPP_PREFIX})
pkg_check_modules(Magickpp QUIET Magick++)
if(NOT Magickpp_FOUND)
    message(FATAL_ERROR "Failed to build Magick++")
endif()
add_library(magickpp-linker INTERFACE)
target_include_directories(magickpp-linker INTERFACE $<BUILD_INTERFACE:${Magickpp_STATIC_INCLUDE_DIRS}>)
target_compile_options(magickpp-linker INTERFACE ${Magickpp_STATIC_CFLAGS_OTHER})
target_static_link(magickpp-linker
    BUILD_LIBRARY_DIRS ${Magickpp_STATIC_LIBRARY_DIRS}
    DEBUG_LIBRARIES ${Magickpp_STATIC_LIBRARIES}
    RELEASE_LIBRARIES ${Magickpp_STATIC_LIBRARIES}
)
target_link_libraries(magickpp-linker INTERFACE libjpeg zlib libpng libwebp)
target_link_directories(magickpp-linker INTERFACE
    $<BUILD_INTERFACE:${MAGICKPP_PREFIX}/lib>
    $<INSTALL_INTERFACE:lib>
)
if(WEBCFACE_SYSTEM_WIN32API)
    target_link_libraries(magickpp-linker INTERFACE urlmon)
endif()
if(Magickpp_VERSION MATCHES "^7\.")
    target_compile_definitions(magickpp-linker INTERFACE WEBCFACE_MAGICK_VER7)
endif()

if(WEBCFACE_INSTALL)
    list(APPEND WEBCFACE_EXPORTS magickpp-linker)
    if(NOT WEBCFACE_SHARED)
        include(cmake/linker.cmake)
        install_prefix_libs(${MAGICKPP_PREFIX}
            LIBRARY_DIRS ${Magickpp_STATIC_LIBRARY_DIRS}
            LIBRARIES ${Magickpp_STATIC_LIBRARIES}
        )
    endif()
    install(FILES
        ${imagemagick_SOURCE_DIR}/LICENSE
        DESTINATION share/webcface/3rd_party/ImageMagick
    )
endif()

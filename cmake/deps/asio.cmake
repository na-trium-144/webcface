include(cmake/fetch.cmake)
option(WEBCFACE_FIND_ASIO "try find_path(asio.hpp)" ${WEBCFACE_FIND_LIBS})

# target = asio (header only)

if(WEBCFACE_FIND_ASIO)
    find_path(ASIO_INCLUDE_DIR asio.hpp)
endif()
if(WEBCFACE_FIND_ASIO AND NOT ASIO_INCLUDE_DIR STREQUAL "ASIO_INCLUDE_DIR-NOTFOUND")
    set(ASIO_FOUND true)
    list(APPEND WEBCFACE_SUMMARY "asio: found at ${ASIO_INCLUDE_DIR}")
else()
    set(ASIO_FOUND false)
    fetch_only(asio
        https://github.com/chriskohlhoff/asio.git
        asio-1-29-0
        asio
    )
    set(ASIO_INCLUDE_DIR ${asio_SOURCE_DIR}/asio/include CACHE INTERNAL "" FORCE)

    if(WEBCFACE_INSTALL)
        install(FILES
            ${asio_SOURCE_DIR}/asio/LICENSE_1_0.txt
            DESTINATION share/webcface/3rd_party/asio
        )
    endif()
endif()

add_library(asio INTERFACE)
target_include_directories(asio INTERFACE $<BUILD_INTERFACE:${ASIO_INCLUDE_DIR}>)
target_compile_definitions(asio INTERFACE ASIO_DISABLE_VISIBILITY)
if(CMAKE_SYSTEM_NAME STREQUAL "CYGWIN" OR CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    # for cygwin
    # todo: curlではWIN32SOCKETを使わないので、干渉する
    target_compile_definitions(asio INTERFACE __USE_W32_SOCKETS _WIN32_WINNT=0x0601)
endif()

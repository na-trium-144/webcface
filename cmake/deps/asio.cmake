include(cmake/fetch.cmake)
option(WEBCFACE_FIND_ASIO "try find_path(asio.hpp)" ${WEBCFACE_FIND_LIBS})

# target = webcface-asio-linker (header only)

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

add_library(webcface-asio-linker INTERFACE)
target_include_directories(webcface-asio-linker INTERFACE $<BUILD_INTERFACE:${ASIO_INCLUDE_DIR}>)
target_compile_definitions(webcface-asio-linker INTERFACE ASIO_DISABLE_VISIBILITY)
if(WEBCFACE_SYSTEM_WIN32SOCKET)
    target_link_libraries(webcface-asio-linker INTERFACE ws2_32 wsock32)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "CYGWIN" OR CMAKE_SYSTEM_NAME STREQUAL "MSYS")
    # for cygwin
    # todo: curlではWIN32SOCKETを使わないので、干渉する
    target_compile_definitions(webcface-asio-linker INTERFACE __USE_W32_SOCKETS _WIN32_WINNT=0x0601)
endif()

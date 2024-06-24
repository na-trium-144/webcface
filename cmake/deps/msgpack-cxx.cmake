include(cmake/fetch.cmake)
option(WEBCFACE_FIND_MSGPACK "try find_package(msgpack-cxx)" ${WEBCFACE_FIND_LIBS})

# target = msgpack-cxx (header only)

if(WEBCFACE_FIND_MSGPACK)
    find_package(msgpack-cxx QUIET)
endif()
if(msgpack-cxx_FOUND)
    list(APPEND WEBCFACE_SUMMARY "msgpack-cxx: ${msgpack-cxx_VERSION} found at ${msgpack-cxx_DIR}")
else()
    # msgpackのcmakelistsを使うとmsgpackをインストールしてしまうので、includeするだけ
    fetch_only(msgpack-cxx
        https://github.com/msgpack/msgpack-c.git
        cpp-6.1.0
        include
    )
    add_library(msgpack-cxx INTERFACE)
    target_compile_definitions(msgpack-cxx INTERFACE
        MSGPACK_NO_BOOST
        MSGPACK_DEFAULT_API_VERSION=3
    )
    target_include_directories(msgpack-cxx INTERFACE $<BUILD_INTERFACE:${msgpack-cxx_SOURCE_DIR}/include>)

    if(WEBCFACE_INSTALL)
        install(FILES
            ${msgpack-c_SOURCE_DIR}/LICENSE_1_0.txt
            DESTINATION share/webcface/3rd_party/msgpack-c
        )
    endif()
endif()

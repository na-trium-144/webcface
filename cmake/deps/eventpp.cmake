include(cmake/fetch.cmake)
option(WEBCFACE_FIND_EVENTPP "try find_package(eventpp)" ${WEBCFACE_FIND_LIBS})

# target = eventpp (header only)

if(WEBCFACE_FIND_EVENTPP)
    find_package(eventpp QUIET)
endif()
if(eventpp_FOUND)
    list(APPEND WEBCFACE_SUMMARY "eventpp: ${eventpp_VERSION} found at ${eventpp_DIR}")

    if(WEBCFACE_INSTALL)
        set(EVENTPP_INSTALLED 1)
    endif()

else()
    set(EVENTPP_INSTALL OFF CACHE INTERNAL "" FORCE)
    fetch_cmake(eventpp
        https://github.com/wqking/eventpp.git
        v0.1.3
    )

    if(WEBCFACE_INSTALL)
        list(APPEND WEBCFACE_EXPORTS eventpp)
        set(EVENTPP_INSTALLED 0)
        install(DIRECTORY ${eventpp_SOURCE_DIR}/include/ DESTINATION include)
        install(FILES
            ${eventpp_SOURCE_DIR}/license
            DESTINATION share/webcface/3rd_party/eventpp
        )
    endif()
endif()

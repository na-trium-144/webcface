include(cmake/fetch.cmake)
option(WEBCFACE_FIND_UTF8CPP "try find_path(utf8.h)" ${WEBCFACE_FIND_LIBS})

# target = utf8cpp (header only)

if(WEBCFACE_FIND_UTF8CPP)
    find_path(UTF8CPP_INCLUDE_DIR utf8.h PATH_SUFFIXES utf8cpp)
endif()
if(WEBCFACE_FIND_UTF8CPP AND NOT UTF8CPP_INCLUDE_DIR STREQUAL "UTF8CPP_INCLUDE_DIR-NOTFOUND")
    set(UTF8CPP_FOUND true)
    list(APPEND WEBCFACE_SUMMARY "utf8cpp: found at ${UTF8CPP_INCLUDE_DIR}")
else()
    set(UTF8CPP_FOUND false)
    fetch_only(utf8cpp
        https://github.com/nemtrif/utfcpp.git
        v4.0.5
        source
    )
    set(UTF8CPP_INCLUDE_DIR ${utf8cpp_SOURCE_DIR}/source)

    if(WEBCFACE_INSTALL)
        install(FILES
            ${utf8cpp_SOURCE_DIR}/LICENSE
            DESTINATION share/webcface/3rd_party/utfcpp
        )
    endif()
endif()

add_library(utf8cpp INTERFACE)
target_include_directories(utf8cpp INTERFACE $<BUILD_INTERFACE:${UTF8CPP_INCLUDE_DIR}>)

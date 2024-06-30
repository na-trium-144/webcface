include(cmake/fetch.cmake)
option(WEBCFACE_FIND_CLI11 "try find_package(CLI11)" ${WEBCFACE_FIND_LIBS})

# target = CLI11 (header only)

if(WEBCFACE_FIND_CLI11)
    find_package(CLI11 QUIET)
endif()
if(CLI11_FOUND)
    list(APPEND WEBCFACE_SUMMARY "cli11: ${CLI11_VERSION} found at ${CLI11_DIR}")
else()
    fetch_cmake(cli11
        https://github.com/CLIUtils/CLI11.git
        v2.3.2
    )

    if(WEBCFACE_INSTALL)
        install(FILES
            ${cli11_SOURCE_DIR}/LICENSE
            DESTINATION share/webcface/3rd_party/cli11
        )
    endif()
endif()

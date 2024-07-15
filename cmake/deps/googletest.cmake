include(cmake/fetch.cmake)
option(WEBCFACE_FIND_GTEST "try find_package(GTest)" ${WEBCFACE_FIND_LIBS})

# target = GTest::gtest_main

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(gtest_force_shared_crt on CACHE INTERNAL "" FORCE)
endif()

if(WEBCFACE_FIND_GTEST)
    find_package(GTest QUIET)
endif()
if(GTest_FOUND)
    list(APPEND WEBCFACE_SUMMARY "googletest: ${GTest_VERSION} found at ${GTest_DIR}")
else()
    set(INSTALL_GTEST off CACHE INTERNAL "" FORCE)
    fetch_cmake(googletest
        https://github.com/google/googletest.git
        v1.14.0
    )
endif()

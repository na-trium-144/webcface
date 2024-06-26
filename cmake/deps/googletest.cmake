include(cmake/fetch.cmake)

# target = GTest::gtest_main
# u8stringの機能を有効にするため、インストールされたものではなくソースからビルドする

set(INSTALL_GTEST off CACHE INTERNAL "" FORCE)
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(gtest_force_shared_crt on CACHE INTERNAL "" FORCE)
endif()
fetch_cmake(googletest
    https://github.com/google/googletest.git
    v1.14.0
)

# based on https://github.com/osm2go/osm2go/blob/master/find_filesystem.cmake .

# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2018 Rolf Eike Beer <eike@sf-mail.de>.

# find out if std::filesystem can be used

include(CMakePushCheckState)
include(CheckCXXSourceRuns)

set(FS_TESTCODE
"
#if defined(CXX17_FILESYSTEM) || defined (CXX17_FILESYSTEM_LIBFS)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif defined(CXX11_EXP_FILESYSTEM) || defined (CXX11_EXP_FILESYSTEM_LIBFS)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#endif

int main(void)
{
    return std_fs::exists(std_fs::path(\"/\")) ? 0 : 1;
}
"
)

# if (CMAKE_CXX_STANDARD LESS 17)
set(OLD_STANDARD "${CMAKE_CXX_STANDARD}")
set(CMAKE_CXX_STANDARD 17)
# endif ()

cmake_push_check_state(RESET)
check_cxx_source_runs("${FS_TESTCODE}" CXX17_FILESYSTEM)
if (NOT CXX17_FILESYSTEM)
    set(CMAKE_REQUIRED_LIBRARIES stdc++fs)
    check_cxx_source_runs("${FS_TESTCODE}" CXX17_FILESYSTEM_LIBFS)
    cmake_reset_check_state()
endif ()

# if (OLD_STANDARD)
# if (NOT CXX17_FILESYSTEM AND NOT CXX17_FILESYSTEM_LIBFS)
set(CMAKE_CXX_STANDARD ${OLD_STANDARD})
# endif ()
unset(OLD_STANDARD)
# endif ()

if (NOT CXX17_FILESYSTEM AND NOT CXX17_FILESYSTEM_LIBFS)
    check_cxx_source_runs("${FS_TESTCODE}" CXX11_EXP_FILESYSTEM)
    if (NOT CXX11_EXP_FILESYSTEM)
        set(CMAKE_REQUIRED_LIBRARIES stdc++fs)
        check_cxx_source_runs("${FS_TESTCODE}" CXX11_EXP_FILESYSTEM_LIBFS)
    endif ()
    # configure_file("${CMAKE_CURRENT_SOURCE_DIR}/filesystem.hxx.in"
    #         "${CMAKE_CURRENT_BINARY_DIR}/filesystem"
    #         @ONLY)
endif ()
cmake_pop_check_state()

unset(FS_TESTCODE)

add_library(webcface-std-filesystem INTERFACE)
if (CXX17_FILESYSTEM_LIBFS OR CXX11_EXP_FILESYSTEM_LIBFS)
    target_link_libraries(webcface-std-filesystem INTERFACE stdc++fs)
endif ()
if(CXX11_EXP_FILESYSTEM OR CXX11_EXP_FILESYSTEM_LIBFS)
    set(WEBCFACE_EXP_FILESYSTEM 1)
endif()

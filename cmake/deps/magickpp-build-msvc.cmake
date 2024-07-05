include(cmake/fetch.cmake)

find_program(DEVENV_COMMAND devenv)
if(DEVENV_COMMAND STREQUAL "DEVENV_COMMAND-NOTFOUND")
    message(FATAL_ERROR "devenv.exe not found")
endif()
find_program(MSBUILD_COMMAND msbuild)
if(MSBUILD_COMMAND STREQUAL "MSBUILD_COMMAND-NOTFOUND")
    message(FATAL_ERROR "msbuild.exe not found")
endif()

# submoduleだとCloneRepositories.cmdがうまくいかない
fetch_only(imagemagick-windows
    https://github.com/ImageMagick/ImageMagick-Windows.git
    35dbf227258caec73fbe4a0a10ea10f2c2c1051d
    Configure
)
# https://learn.microsoft.com/en-us/cpp/overview/compiler-versions?view=msvc-170
# https://stackoverflow.com/questions/33380128/visual-studio-2015-command-line-retarget-solution
if(CMAKE_CXX_COMPILER_VERSION STRGREATER "19.30")
    set(VS_TOOLCHAIN v143)
    set(VS_VERSION 2022)
elseif(CMAKE_CXX_COMPILER_VERSION STRGREATER "19.20")
    set(VS_TOOLCHAIN v142)
    set(VS_VERSION 2019)
elseif(CMAKE_CXX_COMPILER_VERSION STRGREATER "19.10")
    set(VS_TOOLCHAIN v141)
    set(VS_VERSION 2017)
else()
    message(FATAL_ERROR "Compiler version ${CMAKE_CXX_COMPILER_VERSION} is older than vs2017(19.10)")
endif()

set(imagemagick_sln IM7.StaticDLL.${CMAKE_C_COMPILER_ARCHITECTURE_ID}.sln)
set(imagemagick_configure Configure/Configure.exe)
if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/${imagemagick_sln} OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick-windows_SOURCE_DIR}/${imagemagick_sln})
    if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/${imagemagick_configure} OR ${CMAKE_CURRENT_LIST_FILE} IS_NEWER_THAN ${imagemagick-windows_SOURCE_DIR}/${imagemagick_configure})
        message(STATUS "Fetching dependencies...")
        execute_process(
            COMMAND CloneRepositories.IM7.cmd
            WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
        )
        # patch
        file(APPEND
            ${imagemagick-windows_SOURCE_DIR}/Projects/MagickCore/magick-baseconfig.h.in
            "\n#pragma warning(disable: 4201)\n"
        )
        message(STATUS "Building Configure...")
        set(COMMAND ${DEVENV_COMMAND} /upgrade Configure.2017.sln)
        message(STATUS "${COMMAND}")
        execute_process(
            COMMAND ${COMMAND}
            WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}/Configure
        )
        set(COMMAND ${MSBUILD_COMMAND} Configure.2017.sln /m /p:PlatformToolset=${VS_TOOLCHAIN})
        message(STATUS "${COMMAND}")
        execute_process(
            COMMAND ${COMMAND}
            WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}/Configure
        )
    endif()
    if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/${imagemagick_configure})
        message(FATAL_ERROR "Failed to build Configure.exe for ImageMagick")
    endif()
    message(STATUS "Executing Configure...")
    set(COMMAND Configure.exe /noWizard /noAliases /noDpc /noHdri /noOpenMP /Q8 /${CMAKE_C_COMPILER_ARCHITECTURE_ID} /VS${VS_VERSION} /smtd)
    message(STATUS "${COMMAND}")
    execute_process(
        COMMAND ${COMMAND}
        WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}/Configure
    )
    set(COMMAND ${DEVENV_COMMAND} /upgrade ${imagemagick_sln})
    message(STATUS "${COMMAND}")
    execute_process(
        COMMAND ${COMMAND}
        WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
    )
endif()
if(NOT EXISTS ${imagemagick-windows_SOURCE_DIR}/${imagemagick_sln})
    message(FATAL_ERROR "Failed to configure ImageMagick")
endif()
message(STATUS "Building ImageMagick...")
set(MAGICKPP_LIB_DIR "${imagemagick-windows_SOURCE_DIR}/Output/lib")
if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
    set(COMMAND ${MSBUILD_COMMAND} ${imagemagick_sln}
        /t:CORE_Magick++
        /m /p:PlatformToolset=${VS_TOOLCHAIN},Configuration=Release,Platform=${CMAKE_C_COMPILER_ARCHITECTURE_ID}
    )
    message(STATUS "${COMMAND}")
    execute_process(
        COMMAND ${COMMAND}
        WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
    )
    file(GLOB MAGICKPP_RL_LIBS
        RELATIVE ${MAGICKPP_LIB_DIR}
        ${MAGICKPP_LIB_DIR}/CORE_RL_*.lib
    )
    if(MAGICKPP_RL_LIBS STREQUAL "")
        message(FATAL_ERROR "Failed to build ImageMagick")
    endif()
    foreach(lib IN LISTS MAGICKPP_RL_LIBS)
        list(APPEND MAGICKPP_LIBS optimized ${MAGICKPP_LIB_DIR}/${lib})
    endforeach()
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR WEBCFACE_CONFIG_ALL)
    set(COMMAND ${MSBUILD_COMMAND} ${imagemagick_sln}
        /t:CORE_Magick++
        /m /p:Configuration=Debug,Platform=${CMAKE_C_COMPILER_ARCHITECTURE_ID}
    )
    message(STATUS "${COMMAND}")
    execute_process(
        COMMAND ${COMMAND}
        WORKING_DIRECTORY ${imagemagick-windows_SOURCE_DIR}
    )
    file(GLOB MAGICKPP_DB_LIBS
        RELATIVE ${MAGICKPP_LIB_DIR}
        ${MAGICKPP_LIB_DIR}/CORE_DB_*.lib
    )
    if(MAGICKPP_DB_LIBS STREQUAL "")
        message(FATAL_ERROR "Failed to build ImageMagick")
    endif()
    foreach(lib IN LISTS MAGICKPP_DB_LIBS)
        list(APPEND MAGICKPP_LIBS debug ${MAGICKPP_LIB_DIR}/${lib})
    endforeach()
endif()

add_library(magickpp-linker INTERFACE)
target_include_directories(magickpp-linker INTERFACE
    $<BUILD_INTERFACE:${imagemagick-windows_SOURCE_DIR}/ImageMagick/Magick++/lib>
    $<BUILD_INTERFACE:${imagemagick-windows_SOURCE_DIR}/ImageMagick>
)
target_link_libraries(magickpp-linker INTERFACE ${MAGICKPP_LIBS})
target_compile_definitions(magickpp-linker INTERFACE STATIC_MAGICK)
target_compile_definitions(magickpp-linker INTERFACE WEBCFACE_MAGICK_VER7)

if(WEBCFACE_INSTALL)
    list(APPEND WEBCFACE_EXPORTS magickpp-linker)
    if(NOT WEBCFACE_SHARED)
        foreach(lib IN LISTS MAGICKPP_DB_LIBS MAGICKPP_RL_LIBS)
            install(FILES ${MAGICKPP_LIB_DIR}/${lib}
                DESTINATION lib
            )
            list(APPEND magickpp_PKGCONFIG_LIBS -l${lib})
        endforeach()
        list(INSERT WEBCFACE_PKGCONFIG_LIBS 0 ${magickpp_PKGCONFIG_LIBS})
    endif()
    install(FILES
        ${imagemagick-windows_SOURCE_DIR}/ImageMagick/LICENSE
        DESTINATION share/webcface/3rd_party/ImageMagick
    )
endif()

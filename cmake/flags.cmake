# magickppなどビルドするプロセスに渡す引数を初期化
macro(init_flags)
    cmake_parse_arguments(INIT_FLAGS
        ""
        ""
        "ARCH"
        ${ARGN}
    )

    unset(WEBCFACE_FLAGS)
    unset(WEBCFACE_LDFLAGS)
    unset(WEBCFACE_CMAKE_PROPS)

    list(APPEND WEBCFACE_FLAGS -O3)
    list(APPEND WEBCFACE_CMAKE_PROPS "-DCMAKE_BUILD_TYPE=Release")

    if(WEBCFACE_PIC)
        list(APPEND WEBCFACE_FLAGS -fPIC)
    endif()
    list(APPEND WEBCFACE_CMAKE_PROPS "-DCMAKE_POSITION_INDEPENDENT_CODE=${WEBCFACE_PIC}")

    if(APPLE)
        if("${INIT_FLAGS_ARCH}" STREQUAL "")
            set(INIT_FLAGS_ARCH "${CMAKE_OSX_ARCHITECTURES}")
        endif()
        foreach(arch IN LISTS INIT_FLAGS_ARCH)
            list(APPEND WEBCFACE_FLAGS -arch ${arch})
            list(APPEND WEBCFACE_LDFLAGS -arch ${arch})
        endforeach()
        # universalビルドにしたい場合もそうでない場合もある
        list(JOIN INIT_FLAGS_ARCH "\;" INIT_FLAGS_ARCH_S)
        list(APPEND WEBCFACE_CMAKE_PROPS "-DCMAKE_OSX_ARCHITECTURES=${INIT_FLAGS_ARCH_S}")
    endif()

    list(JOIN CMAKE_PREFIX_PATH_S "\;" CMAKE_PREFIX_PATH_S)
    list(APPEND WEBCFACE_CMAKE_PROPS "-DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_S}")
    list(JOIN WEBCFACE_FLAGS " " WEBCFACE_FLAGS)
    list(JOIN WEBCFACE_LDFLAGS " " WEBCFACE_LDFLAGS)
endmacro()

# DIRECTORIESのそれぞれにあるFILESのそれぞれに対してlipoでユニバーサルライブラリを生成して
# DESTINATIONに出力
macro(lipo)
    cmake_parse_arguments(LIPO
        ""
        "DESTINATION"
        "DIRECTORIES;FILES"
        ${ARGN}
    )
    find_program(LIPO_COMMAND lipo)
    foreach(file IN LISTS LIPO_FILES)
        set(sources "")
        foreach(dir IN LISTS LIPO_DIRECTORIES)
            list(APPEND sources ${dir}/${file})
        endforeach()
        message(STATUS "lipo: ${sources} -> ${LIPO_DESTINATION}/${file}")
        execute_process(COMMAND ${LIPO_COMMAND} -create ${sources} -output ${LIPO_DESTINATION}/${file})
    endforeach()
endmacro()

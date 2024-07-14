if(TARGET opencv_core)
    set(OpenCV_FOUND 1)
endif()

list(APPEND EXAMPLE_SRC
    value.cc
    func.cc
    log.cc
    view.cc
    canvas2d.cc
    canvas3d.cc
    recv.cc
    c_example.c
    benchmark.cc
    image-send.cc
    image-recv.cc
)
if(OpenCV_FOUND)
    list(APPEND EXAMPLE_SRC
        cv-send.cc
        cv-recv.cc
    )
endif()

foreach(SRC IN LISTS EXAMPLE_SRC)
    get_filename_component(TARGET ${SRC} NAME_WE)
    add_executable(webcface-example-${TARGET} ${SRC})
    if(WEBCFACE_SYSTEM_VERSION_RC)
        target_sources(webcface-example-${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../version.rc)
    endif()
    target_link_libraries(webcface-example-${TARGET} PRIVATE webcface)

    set(WEBCFACE_EXAMPLE_TARGETS_CPACK
        ${WEBCFACE_EXAMPLE_TARGETS_CPACK}
        webcface-example-${TARGET} "Example ${TARGET}"
    )
    set_target_properties(webcface-example-${TARGET} PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
    target_compile_definitions(webcface-example-${TARGET} PRIVATE
        FILE_DESCRIPTION="WebCFace Example ${TARGET}"
        FILE_IS_APP
    )

    if(WEBCFACE_INSTALL)
        install(TARGETS webcface-example-${TARGET} RUNTIME DESTINATION bin)
    endif()
endforeach()

foreach(TARGET image-recv image-send)
    target_link_libraries(webcface-example-${TARGET} PRIVATE
        magickpp-linker
    )
endforeach()

if(OpenCV_FOUND)
    foreach(TARGET cv-recv cv-send)
        target_link_libraries(webcface-example-${TARGET} PRIVATE
            opencv_core
            opencv_imgcodecs
            opencv_imgproc
            opencv_highgui
        )
    endforeach()
endif()

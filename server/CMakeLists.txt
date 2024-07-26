add_executable(webcface-server server_main.cc)
if(WIN32)
    target_sources(webcface-server PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../version.rc)
endif()
target_link_libraries(webcface-server PRIVATE
    webcface
    webcface-server-store
    CLI11::CLI11
)
set_target_properties(webcface-server PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib/;$ORIGIN/;@loader_path/../lib/;@loader_path/"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)
target_compile_definitions(webcface-server PRIVATE
    FILE_DESCRIPTION="WebCFace Server"
    FILE_IS_APP
)

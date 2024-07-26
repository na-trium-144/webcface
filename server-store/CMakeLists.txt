# server-store: public
add_library(webcface-server-store STATIC)
target_sources(webcface-server-store PRIVATE
    src/member_data.cc
    src/image_conversion.cc
    src/store.cc
    src/websock.cc
    src/dir.cc
    src/ip.cc
    src/server_ws.cc
)
target_include_directories(webcface-server-store PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(webcface-server-store PRIVATE WEBCFACE_BUILDING)
target_compile_features(webcface-server-store PUBLIC cxx_std_17)
target_link_libraries(webcface-server-store PUBLIC
    webcface-message
    webcface-encoding
    webcface-std-filesystem
    webcface-spdlog-linker
)
target_link_libraries(webcface-server-store PRIVATE
    webcface-client
    webcface-magickpp-linker
    webcface-crow-linker
)
target_link_directories(webcface-server-store PUBLIC
    $<TARGET_PROPERTY:webcface-magickpp-linker,INTERFACE_LINK_DIRECTORIES>
)

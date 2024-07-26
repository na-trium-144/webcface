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
target_include_directories(webcface-server-store PRIVATE
    $<TARGET_PROPERTY:webcface-encoding,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:webcface-client,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:webcface-message,INTERFACE_INCLUDE_DIRECTORIES>
)
target_compile_features(webcface-server-store PUBLIC cxx_std_17)
target_link_libraries(webcface-server-store PUBLIC
    webcface
    webcface-std-filesystem
    webcface-spdlog-linker
)
target_link_libraries(webcface-server-store PRIVATE
    webcface-magickpp-linker
    webcface-crow-linker
    msgpack-cxx
)
target_link_directories(webcface-server-store PUBLIC
    $<TARGET_PROPERTY:webcface-magickpp-linker,INTERFACE_LINK_DIRECTORIES>
)

# server-store: public
add_library(webcface-server-store OBJECT)
target_sources(webcface-server-store PRIVATE
    src/member_data.cc
    src/image_conversion.cc
    src/store.cc
    src/websock.cc
    src/dir.cc
    src/ip.cc
    src/unix_path.cc
)
target_include_directories(webcface-server-store PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(webcface-server-store PRIVATE WEBCFACE_BUILDING)
target_compile_features(webcface-server-store PRIVATE cxx_std_17)
target_link_libraries(webcface-server-store PRIVATE
    webcface-server-internal
    webcface-encoding
    webcface-message
    magickpp-linker
    spdlog-linker
)

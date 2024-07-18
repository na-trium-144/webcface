# message: APIはprivateだがシンボルは公開
add_library(webcface-message OBJECT)
target_sources(webcface-message PRIVATE
    src/message.cc
)
target_include_directories(webcface-message PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(webcface-message PRIVATE WEBCFACE_BUILDING)
target_compile_features(webcface-message PUBLIC cxx_std_17)
target_link_libraries(webcface-message PUBLIC
    webcface-encoding
    msgpack-cxx
    webcface-utf8cpp-linker
    webcface-spdlog-linker
)

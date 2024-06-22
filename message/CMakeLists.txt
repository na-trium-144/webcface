# message: APIはprivateだがシンボルは公開
add_library(webcface-message OBJECT)
target_sources(webcface-message PRIVATE
    src/message.cc
)
target_include_directories(webcface-message PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(webcface-message PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-message PUBLIC
    webcface-encoding
    msgpack-cxx
    utf8cpp
    spdlog-linker
)

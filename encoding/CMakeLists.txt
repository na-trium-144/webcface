# encoding: public
add_library(webcface-encoding OBJECT)
target_sources(webcface-encoding PRIVATE
    src/encoding.cc
    src/val_adaptor.cc
)
target_include_directories(webcface-encoding PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(webcface-encoding PRIVATE WEBCFACE_BUILDING)
target_compile_features(webcface-encoding PRIVATE cxx_std_17)
target_link_libraries(webcface-encoding PRIVATE utf8cpp)

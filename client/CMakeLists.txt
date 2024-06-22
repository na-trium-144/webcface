# client: public
add_library(webcface-client OBJECT)
target_sources(webcface-client PRIVATE
    src/client.cc
    src/client_ws.cc
    src/data_store1.cc
    src/data_store2.cc
    src/field.cc
    src/func_info.cc
    src/func_result.cc
    src/func.cc
    src/func_listener.cc
    src/log.cc
    src/logger.cc
    src/member.cc
    src/text.cc
    src/robot_link.cc
    src/robot_model.cc
    src/canvas3d.cc
    src/canvas2d.cc
    src/value.cc
    src/view.cc
    src/image.cc
    src/image_frame.cc
    src/version.cc
    src/component_canvas3d.cc
    src/component_canvas2d.cc
    src/component_view.cc
    src/c_wcf/client.cc
    src/c_wcf/value.cc
    src/c_wcf/text.cc
    src/c_wcf/func.cc
    src/c_wcf/view.cc
)
target_include_directories(webcface-client PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)
target_compile_definitions(webcface-client PRIVATE WEBCFACE_BUILDING)
target_link_libraries(webcface-client PRIVATE
    webcface-message
    webcface-server-store
    libcurl-linker
)
target_link_libraries(webcface-client PUBLIC
    webcface-encoding
    eventpp::eventpp
    spdlog-linker
)

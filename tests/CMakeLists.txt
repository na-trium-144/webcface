add_executable(webcface-test
    client_data_test.cc
    data_test.cc
    func_test.cc
    func_listener_test.cc
    view_test.cc
    robot_model_test.cc
    canvas3d_test.cc
    canvas2d_test.cc
    image_test.cc
    member_test.cc
    logger_test.cc
    client_test.cc
    dummy_server.cc
    dummy_client.cc
    server_test.cc
    encoding_test.cc
    c_wcf_test.cc
)
target_link_libraries(webcface-test PRIVATE
    webcface
    GTest::gtest_main
    msgpack-cxx
    utf8cpp
    eventpp::eventpp
    spdlog-linker
    magickpp-linker
    crow-linker
    libcurl-linker
)
target_compile_definitions(webcface-test PRIVATE
    WEBCFACE_TEST_TIMEOUT=${WEBCFACE_TEST_TIMEOUT}
)
target_include_directories(webcface-test PRIVATE
    $<TARGET_PROPERTY:webcface-encoding,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:webcface-server-store,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:webcface-client,INTERFACE_INCLUDE_DIRECTORIES>
    $<TARGET_PROPERTY:webcface-message,INTERFACE_INCLUDE_DIRECTORIES>
)

include(GoogleTest)
gtest_discover_tests(webcface-test DISCOVERY_TIMEOUT 600)

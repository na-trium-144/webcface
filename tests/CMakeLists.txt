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
    webcface-encoding
    webcface-message
    webcface-server-store
    webcface
    GTest::gtest_main
    msgpack-cxx
    utf8cpp
    eventpp::eventpp
    spdlog-linker
    libcurl-linker
    crow-linker
)
target_compile_definitions(webcface-test PRIVATE
    WEBCFACE_TEST_TIMEOUT=${WEBCFACE_TEST_TIMEOUT}
)
include(GoogleTest)
gtest_discover_tests(webcface-test DISCOVERY_TIMEOUT 600)

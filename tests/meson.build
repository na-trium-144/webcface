test_src = [
  'client_data_test.cc',
  'data_test.cc',
  'func_test.cc',
  'func_listener_test.cc',
  'view_test.cc',
  'robot_model_test.cc',
  'canvas3d_test.cc',
  'canvas2d_test.cc',
  'image_test.cc',
  'member_test.cc',
  'logger_test.cc',
  'encoding_test.cc',
]
test_dependencies = [
  warning_options,
  webcface_dep,
  webcface_encoding_dep,
  webcface_message_dep,
  webcface_client_dep,
  gtest_dep,
  gtest_main_dep,
  msgpack_cxx_dep,
  utf8cpp_dep,
  spdlog_dep,
]
if build_server
  test_src += [
    'client_test.cc',
    'dummy_server.cc',
    'dummy_client.cc',
    'server_test.cc',
    'c_wcf_test.cc',
  ]
  test_dependencies += [
    webcface_server_store_dep,
    magickpp_dep,
    crow_dep,
    curl_dep,
  ]
endif
webcface_test = executable('webcface-test',
  sources: test_src,
  cpp_args: [
    '-DWEBCFACE_TEST_TIMEOUT=' + get_option('test_wait').to_string(),
  ],
  dependencies: test_dependencies,
)
# test('webcface-test', webcface_test,
#   env: {
#     'WEBCFACE_TRACE': '1',
#   },
#   protocol: 'gtest',
#   verbose: true,
#   timeout: 
# )

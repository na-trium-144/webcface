test_src = [
  'client_data_test.cc',
  'value_test.cc',
  'text_test.cc',
  'log_test.cc',
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
  'transform_test.cc',
]
test_dependencies = [
  warning_options,
  # webcface_dep,
  webcface_config_dep,
  webcface_common_dep,
  webcface_client_dep,
  gtest_dep,
  msgpack_cxx_dep,
  utf8cpp_dep,
  spdlog_dep,
]
test_link_libs = [
  webcface_common_lib,
  webcface_client_lib,
]
if build_server
  test_src += [
    'client_test.cc',
    'client_test_2.cc',
    'client_test_3.cc',
    'dummy_server.cc',
    'dummy_client.cc',
    'server_test.cc',
    'server_test_2.cc',
    'c_wcf_test.cc',
  ]
  test_dependencies += [
    webcface_server_store_dep,
    libvips_cpp_dep,
    crow_dep,
    curl_dep,
  ]
  test_link_libs += [
    webcface_server_store_lib,
  ]
endif
test_link_args = []
if host_machine.system() == 'windows' and cxx.get_argument_syntax() != 'msvc'
  # https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean
  test_link_args += '-Wl,--default-image-base-low'
endif
webcface_test = executable('webcface-test',
  sources: test_src,
  cpp_args: [
    '-DWEBCFACE_BUILDING',
    '-DWEBCFACE_TEST_TIMEOUT=' + get_option('test_wait').to_string(),
  ],
  link_args: test_link_args,
  dependencies: test_dependencies,
  link_with: test_link_libs,
)
summary('Test wait (ms)', get_option('test_wait'))
test_parsed = run_command(py,
  meson.project_source_root() / 'scripts' / 'add_tests.py',
  test_src,
  check: true,
).stdout().strip().split()
summary('Test cases', test_parsed.length())
foreach case: test_parsed
  test(case.split('.')[1], webcface_test,
    suite: case.split('.')[0],
    args: [
      '--gtest_filter=' + case,
      '--gtest_catch_exceptions=0',
      '--gtest_break_on_failure',
    ],
    is_parallel: false,
    env: {
      'WEBCFACE_TRACE': '1',
    },
    protocol: 'gtest',
    timeout: get_option('test_wait') * 2 / 10,
  )
endforeach

benchmark_src = [
  'main.cc',
  'sync-value.cc',
  'sync-text.cc',
  'latency-value.cc',
  'latency-text.cc',
]
test_dependencies = [
  warning_options_dep,
  # webcface_dep,
  webcface_config_dep,
  webcface_common_dep,
  webcface_client_dep,
  benchmark_dep,
  msgpack_cxx_dep,
  utf8cpp_dep,
  spdlog_dep,
  webcface_server_store_dep,
  libvips_cpp_dep,
  crow_dep,
  curl_dep,
]
test_link_libs = [
  webcface_common_lib,
  webcface_client_lib,
  webcface_server_store_lib,
]
test_link_args = []
if host_machine.system() == 'windows' and cxx.get_argument_syntax() != 'msvc'
  # https://stackoverflow.com/questions/10486116/what-does-this-gcc-error-relocation-truncated-to-fit-mean
  test_link_args += '-Wl,--default-image-base-low'
endif

webcface_benchmark = executable('webcface-benchmark',
  sources: benchmark_src,
  dependencies: test_dependencies,
  link_with: test_link_libs,
  link_args: test_link_args,
  cpp_args: [
    '-DWEBCFACE_BUILDING',
  ],
)
benchmark('webcface-benchmark', webcface_benchmark,
  timeout: 120,
)

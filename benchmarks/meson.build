benchmark_src = [
  'main.cc',
  'sync-value.cc',
  'sync-text.cc',
  'latency-value.cc',
  'latency-text.cc',
]
test_dependencies = [
  warning_options_dep,
  webcface_dep,
  benchmark_dep,
  webcface_server_store_dep,
]
test_link_libs = [
  webcface_common_lib,
  webcface_server_store_lib,
]

webcface_benchmark = executable('webcface-benchmark',
  sources: benchmark_src,
  dependencies: test_dependencies,
  link_with: test_link_libs,
)
benchmark('webcface-benchmark', webcface_benchmark,
  timeout: 120,
)

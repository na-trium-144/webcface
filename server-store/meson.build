webcface_server_store_src = [
  'src/member_data.cc',
  'src/image_conversion.cc',
  'src/store.cc',
  'src/websock.cc',
  'src/dir.cc',
  'src/ip.cc',
  'src/server_ws.cc',
]
webcface_server_store_inc = include_directories('include')
webcface_server_store_lib = static_library('webcface-server-store',
  sources: webcface_server_store_src,
  include_directories: [
    webcface_server_store_inc,
  ],
  dependencies: [
    webcface_config_dep,
    warning_options_dep,
    webcface_common_dep,
    filesystem_dep,
    spdlog_dep,
    libvips_cpp_dep,
    crow_dep,
    win32socket_deps,
  ],
  cpp_args: ['-DWEBCFACE_BUILDING'],
)
webcface_server_store_dep = declare_dependency(
  include_directories: [
    webcface_server_store_inc,
  ],
  dependencies: [
    webcface_config_dep,
    webcface_common_dep,
    filesystem_dep,
    spdlog_dep,
    libvips_cpp_dep,
    win32socket_deps,
  ],
)

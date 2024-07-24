webcface_server_store_src = [
  'src/member_data.cc',
  'src/image_conversion.cc',
  'src/store.cc',
  'src/websock.cc',
  'src/dir.cc',
  'src/ip.cc',
  'src/unix_path.cc',
]
webcface_server_store_inc = include_directories('include')
webcface_server_store_lib = static_library('webcface-server-store',
  sources: webcface_server_store_src,
  include_directories: [
    webcface_config_inc,
    webcface_server_store_inc,
  ],
  dependencies: [
    webcface_encoding_dep,
    webcface_message_dep,
    webcface_server_internal_dep,
    spdlog_dep,
    magickpp_dep,
  ],
  build_by_default: false,
  cpp_args: ['-DWEBCFACE_BUILDING'],
)
webcface_server_store_dep = declare_dependency(
  include_directories: [
    webcface_server_store_inc,
  ],
  dependencies: [
    webcface_encoding_dep,
    webcface_message_dep,
    spdlog_dep,
  ],
)

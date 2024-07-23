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
    webcface_encoding_inc,
    webcface_message_inc,
    webcface_server_internal_inc,
    webcface_server_store_inc,
  ],
  build_by_default: false,
  cpp_args: ['-DWEBCFACE_BUILDING'],
)

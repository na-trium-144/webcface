webcface_message_src = [
  'src/message.cc',
]
webcface_message_inc = include_directories('include')
webcface_message_lib = static_library('webcface-message',
  sources: webcface_message_src,
  include_directories: [
    webcface_config_inc,
    webcface_message_inc,
  ],
  dependencies: [
    warning_options,
    webcface_encoding_dep,
    msgpack_cxx_dep,
    spdlog_dep,
    utf8cpp_dep,
  ],
  build_by_default: false,
  cpp_args: ['-DWEBCFACE_BUILDING'],
  gnu_symbol_visibility: 'inlineshidden',
)
webcface_message_dep = declare_dependency(
  include_directories: [
    webcface_message_inc,
  ],
  dependencies: [
    webcface_encoding_dep,
    msgpack_cxx_dep,
    spdlog_dep,
    utf8cpp_dep,
  ],
)

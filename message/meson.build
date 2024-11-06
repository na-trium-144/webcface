webcface_message_src = [
  'src/base.cc',
  'src/canvas2d.cc',
  'src/canvas3d.cc',
  'src/func.cc',
  'src/image.cc',
  'src/log.cc',
  'src/robot_model.cc',
  'src/sync.cc',
  'src/text.cc',
  'src/value.cc',
  'src/view.cc',
]
webcface_message_inc = include_directories('include')
webcface_message_lib = static_library('webcface-message',
  sources: webcface_message_src,
  include_directories: [
    webcface_message_inc,
  ],
  dependencies: [
    warning_options,
    webcface_config_dep,
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

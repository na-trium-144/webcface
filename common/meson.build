webcface_common_src = [
  'src/encoding.cc',
  'src/val_adaptor.cc',
  'src/message.cc',
  'src/unix_path.cc',
  'src/message/canvas2d.cc',
]
webcface_common_inc = include_directories('include')
webcface_common_lib = static_library('webcface-common',
  sources: webcface_common_src,
  include_directories: [
    webcface_common_inc,
  ],
  dependencies: [
    webcface_config_dep,
    warning_options,
    mpack_dep,
    spdlog_dep,
    utf8cpp_dep,
    win32folders_dep,
  ],
  build_by_default: false,
  cpp_args: ['-DWEBCFACE_BUILDING'],
  gnu_symbol_visibility: 'inlineshidden',
)
webcface_common_dep = declare_dependency(
  include_directories: [
    webcface_common_inc,
  ],
  dependencies: [
    mpack_dep,
    spdlog_dep,
    utf8cpp_dep,
  ],
)

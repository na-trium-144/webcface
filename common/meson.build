webcface_common_src = [
  'src/encoding.cc',
  'src/fmt.cc',
  'src/num_vector.cc',
  'src/val_adaptor.cc',
  'src/message.cc',
  'src/unix_path.cc',
  'src/smr.cc',
]
webcface_common_inc = include_directories('include')
webcface_common_lib = static_library('webcface-common',
  sources: webcface_common_src,
  include_directories: [
    webcface_common_inc,
  ],
  dependencies: [
    webcface_config_dep,
    wabi_options_dep,
    warning_options_dep,
    msgpack_cxx_dep,
    spdlog_dep,
    fmt_dep,
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
    msgpack_cxx_dep,
    spdlog_dep,
    fmt_dep,
    utf8cpp_dep,
  ],
)

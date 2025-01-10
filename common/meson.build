webcface_common_src = [
  'src/encoding.cc',
  'src/fmt.cc',
  'src/val_adaptor.cc',
  'src/message.cc',
  'src/unix_path.cc',
]
webcface_common_inc = include_directories('include')
webcface_unity_args = []
if get_option('unity') == 'on' or (get_option('unity') == 'subprojects' and meson.is_subproject())
  webcface_unity_args += '-DWEBCFACE_UNITY'
endif
webcface_common_lib = static_library('webcface-common',
  sources: webcface_common_src,
  include_directories: [
    webcface_common_inc,
  ],
  dependencies: [
    webcface_config_dep,
    warning_options,
    msgpack_cxx_dep,
    spdlog_dep,
    fmt_dep,
    utf8cpp_dep,
    win32folders_dep,
  ],
  build_by_default: false,
  cpp_args: [
    '-DWEBCFACE_BUILDING',
    webcface_unity_args,
  ],
  gnu_symbol_visibility: 'inlineshidden',
)
webcface_common_dep = declare_dependency(
  include_directories: [
    webcface_common_inc,
  ],
  compile_args: [
    webcface_unity_args,
  ],
  dependencies: [
    msgpack_cxx_dep,
    spdlog_dep,
    fmt_dep,
    utf8cpp_dep,
  ],
)

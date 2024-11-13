webcface_common_src = [
  'src/encoding.cc',
  'src/val_adaptor.cc',
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
    utf8cpp_dep,
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
  ],
)

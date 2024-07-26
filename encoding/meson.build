webcface_encoding_src = [
  'src/encoding.cc',
  'src/val_adaptor.cc',
]
webcface_encoding_inc = include_directories('include')
webcface_encoding_lib = static_library('webcface-encoding',
  sources: webcface_encoding_src,
  include_directories: [
    webcface_config_inc,
    webcface_encoding_inc,
  ],
  dependencies: [
    utf8cpp_dep,
  ],
  build_by_default: false,
  cpp_args: ['-DWEBCFACE_BUILDING'],
  gnu_symbol_visibility: 'inlineshidden',
)
webcface_encoding_dep = declare_dependency(
  include_directories: [
    webcface_encoding_inc,
  ],
  dependencies: [
  ],
)

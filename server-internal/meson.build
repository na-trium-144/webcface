webcface_server_internal_src = [
  'src/server_ws.cc',
]
webcface_server_internal_inc = include_directories('include')
webcface_server_internal_lib = static_library('webcface-server-internal',
  sources: webcface_server_internal_src,
  include_directories: [
    webcface_server_internal_inc,
  ],
  dependencies: [
  ],
)
webcface_server_internal_dep = declare_dependency(
  include_directories: [
    webcface_server_internal_inc,
  ],
  dependencies: [
  ],
)

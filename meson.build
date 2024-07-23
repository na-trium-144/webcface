project('webcface', 'c', 'cpp',
  version: '2.0.0',
  default_options: [
    'buildtype=release',
    'c_std=c99',
    'cpp_std=c++17',
  ]
)
fs = import('fs')
cmake = import('cmake')

assert(
  get_option('default_library') == 'shared' or get_option('default_library') == 'static',
  'building both shared and static webcface libraries is currently not supported',
)
conf_data = configuration_data({
  'WEBCFACE_VERSION_MAJOR': 2,
  'WEBCFACE_VERSION_MINOR': 0,
  'WEBCFACE_VERSION_REVISION': 0,
  'WEBCFACE_VERSION': '2.0.0',
})
conf_data.set10('WEBCFACE_SHARED', get_option('default_library') == 'shared')
configure_file(
  input: 'config.h.in',
  output: 'webcface-config.h',
  configuration: conf_data
)
add_project_arguments(
  '-DWEBCFACE_MESON',
  language: ['c', 'cpp'],
)
webcface_config_inc = include_directories('.')

msgpack_cxx_dep = dependency('msgpack-cxx', method: 'cmake', required: false)
if not msgpack_cxx_dep.found()
  if fs.exists(meson.global_source_root() / 'subprojects/git_msgpack-c/CMakeLists.txt')
    opt_var = cmake.subproject_options()
    opt_var.add_cmake_defines({
      'MSGPACK_USE_BOOST': false,
    })
    msgpack_cxx_proj = cmake.subproject('git_msgpack-c', options: opt_var)
    msgpack_cxx_dep = msgpack_cxx_proj.dependency('msgpack-cxx')
  else
    msgpack_cxx_proj = subproject('msgpackc-cxx')
    msgpack_cxx_dep = msgpack_cxx_proj.get_variable('msgpack_cxx_dep')
  endif
endif

subdir('encoding')
subdir('message')
subdir('server-internal')
subdir('server-store')
subdir('client')

library('webcface',
  objects: [
    webcface_encoding_lib.extract_all_objects(recursive: false),
    webcface_message_lib.extract_all_objects(recursive: false),
    webcface_server_store_lib.extract_all_objects(recursive: false),
    webcface_client_lib.extract_all_objects(recursive: false),
  ],
  link_with: [
    webcface_server_internal_lib,
  ],
  version: '2.0.0',
  soversion: '20',
)

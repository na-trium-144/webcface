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

msgpack_cxx_dep = dependency('msgpack-cxx', required: false)
if not msgpack_cxx_dep.found()
  msgpack_cxx_dep = subproject('msgpackc-cxx').get_variable('msgpack_cxx_dep')
endif

spdlog_dep = dependency('spdlog', required: false)
if not spdlog_dep.found()
  spdlog_dep = subproject('spdlog').get_variable('spdlog_dep')
endif

utf8cpp_dep = dependency('utf8cpp', required: false)
if not utf8cpp_dep.found()
  utf8cpp_dep = subproject('utfcpp').get_variable('utfcpp_dep')
endif

crow_dep = dependency('crow', required: false)
if not crow_dep.found()
  crow_dep = subproject('crow').get_variable('crow_dep')
endif

magickpp_dep = dependency('Magick++', required: false)
if not magickpp_dep.found()
  magickpp_dep = subproject('imagemagick').get_variable('magickpp_dep')
endif

curl_dep = dependency('libcurl', required: false)
if not curl_dep.found()
  curl_dep = subproject('curl').get_variable('curlu_dep')
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

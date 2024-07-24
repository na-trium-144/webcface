project('webcface', 'c', 'cpp',
  version: '2.0.0',
  license: 'MIT',
  meson_version: '>=0.55.0',
  default_options: [
    'buildtype=release',
    'c_std=c99',
    'cpp_std=c++17',
  ],
)
fs = import('fs')
cmake = import('cmake')

webcface_soversion = '20'
webcface_version_str = meson.project_version()

assert(
  get_option('default_library') == 'shared' or get_option('default_library') == 'static',
  'building both shared and static webcface libraries is currently not supported',
)
conf_data = configuration_data({
  'WEBCFACE_VERSION_MAJOR': meson.project_version().split('.')[0],
  'WEBCFACE_VERSION_MINOR': meson.project_version().split('.')[1],
  'WEBCFACE_VERSION_REVISION': meson.project_version().split('.')[2],
  'WEBCFACE_VERSION': webcface_version_str,
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

msgpack_cxx_dep = dependency('msgpack-cxx',
  default_options: [
    'boost=disabled',
    'tests=disabled',
  ],
)
spdlog_dep = dependency('spdlog',
  default_options: [
    'tests=disabled',
    'compile_library=true',
    'default_library=static',
    'external_fmt=disabled',
    'std_format=disabled',
  ],
)
utf8cpp_dep = dependency('utf8cpp')
crow_dep = dependency('Crow')
magickpp_dep = dependency('Magick++')
curl_dep = dependency('libcurl',
  default_options: [
    'websockets=enabled',
    'default_library=static',
    'tool=disabled',
    'tests=disabled',
    'unittests=disabled',
    'bindlocal=disabled',
    'brotli=disabled',
    'doh=disabled',
    'form-api=disabled',
    'getoptions=disabled',
    'gsasl=disabled',
    'libcurl-option=disabled',
    'libz=disabled',
    'netrc=disabled',
    'parsedate=disabled',
    'progress-meter=disabled',
    'psl=disabled',
    'sspi=disabled',
    'zstd=disabled',
    'gss-api=disabled',
    'idn=disabled',
    'ntlm=disabled',
    'ssh=disabled',
    'ssl=disabled',
    'tls-srp=disabled',
    'openssl=disabled',
    'schannel=disabled',
    'secure-transport=disabled',
    'dict=disabled',
    'file=disabled',
    'ftp=disabled',
    'gopher=disabled',
    'imap=disabled',
    'ldap=disabled',
    'ldaps=disabled',
    'mqtt=disabled',
    'pop3=disabled',
    'rtmp=disabled',
    'rtsp=disabled',
    'smb=disabled',
    'smtp=disabled',
    'telnet=disabled',
    'tftp=disabled',
  ],
)

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
  dependencies: [
    spdlog_dep,
    magickpp_dep,
    curl_dep,
  ],
  version: webcface_version_str,
  soversion: webcface_soversion,
)

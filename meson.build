project('webcface', 'c', 'cpp',
  version: '2.9.0',
  license: 'MIT',
  meson_version: '>=1.3.0',
  default_options: [
    'warning_level=3', # everythingは現在たぶん通らない
    'werror=true',
    # 'c_std=c99', # 指定するとlibjpegのビルドが通らない
    'cpp_std=gnu++17,c++17', # spdlogがgnu++必要、webcfaceは17以上必須
    'default_library=shared',

    # options for subproject
    'msgpackc-cxx:boost=disabled',
    'msgpackc-cxx:tests=disabled',
    'msgpackc-cxx:std_variant=false', # msvcでなんかエラーになる
    'fmt:default_library=static',
    'fmt:warning_level=0',
    'fmt:werror=false',
    'spdlog:tests=disabled',
    'spdlog:compile_library=true',
    'spdlog:default_library=static',
    'spdlog:std_format=disabled',
    'spdlog:werror=false',
    'curl:warning_level=0',
    'curl:werror=false',
    'curl:websockets=enabled',
    'curl:default_library=static',
    'curl:tool=disabled',
    'curl:tests=disabled',
    'curl:unittests=disabled',
    'curl:bindlocal=disabled',
    'curl:brotli=disabled',
    'curl:doh=disabled',
    'curl:form-api=disabled',
    'curl:getoptions=disabled',
    'curl:gsasl=disabled',
    'curl:http2=disabled',
    'curl:ipv6=disabled',
    'curl:libcurl-option=disabled',
    'curl:libz=disabled',
    'curl:netrc=disabled',
    'curl:parsedate=disabled',
    'curl:progress-meter=disabled',
    'curl:psl=disabled',
    'curl:sspi=disabled',
    'curl:zstd=disabled',
    'curl:gss-api=disabled',
    'curl:idn=disabled',
    'curl:ntlm=disabled',
    'curl:ssh=disabled',
    'curl:ssl=disabled',
    'curl:tls-srp=disabled',
    'curl:openssl=disabled',
    'curl:schannel=disabled',
    'curl:secure-transport=disabled',
    'curl:dict=disabled',
    'curl:file=disabled',
    'curl:ftp=disabled',
    'curl:gopher=disabled',
    'curl:imap=disabled',
    'curl:ldap=disabled',
    'curl:ldaps=disabled',
    'curl:mqtt=disabled',
    'curl:pop3=disabled',
    'curl:rtmp=disabled',
    'curl:rtsp=disabled',
    'curl:smb=disabled',
    'curl:smtp=disabled',
    'curl:telnet=disabled',
    'curl:tftp=disabled',
    'libvips:default_library=static',
    'libvips:warning_level=0',
    'libvips:werror=false',
    'libvips:jpeg=enabled',
    'libvips:spng=enabled',
    'libvips:webp=enabled',
    'libvips:zlib=enabled',
    'libvips:man=false',
    'libvips:po=false',
    'libvips:tools=false',
    'libvips:test=false',
    'libvips:fuzz=false',
    'libvips:deprecated=false',
    'libvips:examples=false',
    'libvips:modules=disabled',
    'libvips:introspection=disabled',
    'libvips:vapi=false',
    'libvips:cfitsio=disabled',
    'libvips:cgif=disabled',
    'libvips:exif=disabled',
    'libvips:fftw=disabled',
    'libvips:fontconfig=disabled',
    'libvips:archive=disabled',
    'libvips:heif=disabled',
    'libvips:heif-module=disabled',
    'libvips:imagequant=disabled',
    'libvips:jpeg-xl=disabled',
    'libvips:jpeg-xl-module=disabled',
    'libvips:lcms=disabled',
    'libvips:magick=disabled',
    'libvips:magick-package=',
    'libvips:magick-module=disabled',
    'libvips:matio=disabled',
    'libvips:nifti=disabled',
    'libvips:openexr=disabled',
    'libvips:openjpeg=disabled',
    'libvips:openslide=disabled',
    'libvips:openslide-module=disabled',
    'libvips:highway=disabled',
    'libvips:orc=disabled',
    'libvips:pangocairo=disabled',
    'libvips:pdfium=disabled',
    'libvips:png=disabled',
    'libvips:poppler=disabled',
    'libvips:poppler-module=disabled',
    'libvips:quantizr=disabled',
    'libvips:rsvg=disabled',
    'libvips:tiff=disabled',
    'libvips:nsgif=false',
    'libvips:ppm=false',
    'libvips:analyze=false',
    'libvips:radiance=false',
    'libvips:unity=off',
    'libjpeg-turbo:default_library=static',
    'libjpeg-turbo:turbojpeg=disabled',
    'libjpeg-turbo:tests=disabled',
    'libjpeg-turbo:warning_level=0',
    'libjpeg-turbo:werror=false',
    'libjpeg-turbo:unity=off',
    'zlib:default_library=static',
    'zlib:tests=disabled',
    'zlib:warning_level=0',
    'zlib:werror=false',
    'zlib:unity=off',
    'spng:default_library=static',
    'spng:warning_level=0',
    'spng:werror=false',
    'spng:build_examples=false',
    'libwebp:default_library=static',
    'libwebp:cwebp=disabled',
    'libwebp:dwebp=disabled',
    'libwebp:webpinfo=disabled',
    'libwebp:webpmux=disabled',
    'libwebp:warning_level=0',
    'libwebp:werror=false',
    'libwebp:unity=off',
    'glib:default_library=static',
    'glib:warning_level=0',
    'glib:werror=false',
    'glib:xattr=false',
    'glib:selinux=disabled',
    'glib:libmount=disabled',
    'glib:man-pages=disabled',
    'glib:dtrace=disabled',
    'glib:systemtap=disabled',
    'glib:sysprof=disabled',
    'glib:tests=false',
    'glib:nls=disabled',
    'glib:glib_debug=disabled',
    'glib:glib_assert=false',
    'glib:glib_checks=false',
    'glib:libelf=disabled',
    'glib:introspection=disabled',
    'glib:unity=off',
    'expat:default_library=static',
    'expat:warning_level=0',
    'expat:werror=false',
    'pcre2:default_library=static',
    'pcre2:warning_level=0',
    'pcre2:werror=false',
    'proxy-libintl:default_library=static',
    'proxy-libintl:warning_level=0',
    'proxy-libintl:werror=false',
    'libffi:default_library=static',
    'libffi:warning_level=0',
    'libffi:werror=false',
    'libffi:unity=off',
    'gtest:default_library=static',
    'google-benchmark:default_library=static',
    'google-benchmark:werror=false',
    'google-benchmark:tests=disabled',
  ],
)
fs = import('fs')
cmake = import('cmake')
cxx = meson.get_compiler('cpp')
py = find_program('python3')

webcface_abi_major = '23' # ABIの破壊的変更で1増やす
webcface_abi_minor = '2' # ABIの追加で1増やす
webcface_webui_version = '1.14.0'

webcface_description = 'Web-based Communication Framework'
webcface_contact = 'na-trium-144@users.noreply.github.com'
webcface_url = 'https://github.com/na-trium-144/webcface'

webcface_version_str = meson.project_version()
if get_option('version_suffix') == 'git'
  git = find_program('git', required: false)
  if git.found()
    git_describe = run_command(git, 'describe', '--tags', check: false)
    if git_describe.returncode() == 0
      webcface_version_str = git_describe.stdout().strip('v').strip()
    else
      warning('Failed to get version from git describe:\n' + git_describe.stderr())
    endif
  endif
elif get_option('version_suffix') != ''
  webcface_version_str = meson.project_version() + '-' + get_option('version_suffix')
endif
summary('Version', webcface_version_str)
summary('ABI Version', webcface_abi_major + '.' + webcface_abi_minor)

assert(
  get_option('default_library') == 'shared' or get_option('default_library') == 'static',
  'building both shared and static webcface libraries is currently not supported',
)
summary('Library', get_option('default_library'))
conf_data = configuration_data({
  'WEBCFACE_VERSION_MAJOR': meson.project_version().split('.')[0],
  'WEBCFACE_VERSION_MINOR': meson.project_version().split('.')[1],
  'WEBCFACE_VERSION_REVISION': meson.project_version().split('.')[2],
  'WEBCFACE_VERSION': '"' + webcface_version_str + '"',
  'WEBCFACE_ABI_MAJOR': webcface_abi_major,
  'WEBCFACE_PTR_SIZE': cxx.sizeof('void*'),
})
conf_data.set10('WEBCFACE_SHARED', get_option('default_library') == 'shared')

webcface_system_visibility = false
webcface_system_dllexport = false
webcface_system_exclude_libs = false
webcface_system_hidden_l = false
webcface_system_wchar_windows = false
webcface_system_win32socket = false
webcface_system_path_windows = false
webcface_system_add_debug = false
webcface_system_version_rc = false
relative_rpath = ''
if host_machine.system() == 'linux'
  summary('System', 'linux')
  webcface_system_visibility = true
  webcface_system_exclude_libs = true
  relative_rpath = '$ORIGIN' / '..' / get_option('libdir')
elif host_machine.system() == 'darwin'
  summary('System', 'darwin')
  webcface_system_visibility = true
  webcface_system_hidden_l = true
  relative_rpath = '@loader_path' / '..' / get_option('libdir')
elif host_machine.system() == 'windows'
  webcface_system_dllexport = true
  webcface_system_wchar_windows = true
  webcface_system_win32socket = true
  webcface_system_path_windows = true
  webcface_system_version_rc = true
  if cxx.get_argument_syntax() == 'msvc'
    summary('System', 'windows msvc ' + host_machine.cpu_family())
    webcface_system_add_debug = true
  else
    summary('System', 'windows mingw')
  endif
elif host_machine.system() == 'cygwin'
  summary('System', 'cygwin')
  if get_option('buildtype') == 'debug'
    add_project_arguments('-Wa,-mbig-obj', language: ['cpp'])
  endif
  webcface_system_dllexport = true
  webcface_system_wchar_windows = true
  webcface_system_version_rc = true
else
  warning('unsupported system: ' + host_machine.system())
endif

webcface_lib_name_release = 'webcface'
webcface_lib_name_debug = 'webcface' + (webcface_system_add_debug ? 'd' : '')
summary('Build type', get_option('buildtype'))
if webcface_system_add_debug
  debug_crt = get_option('b_vscrt') == 'mtd' or get_option('b_vscrt') == 'mdd' or \
    (get_option('b_vscrt').endswith('from_buildtype') and get_option('buildtype') == 'debug')
  if debug_crt
    webcface_lib_name = webcface_lib_name_debug
  else
    webcface_lib_name = webcface_lib_name_release
  endif
  summary('Debug CRT', debug_crt, bool_yn: true)
else
  webcface_lib_name = webcface_lib_name_release
endif

if cxx.get_id() == 'gcc' and cxx.get_argument_syntax() != 'msvc'
  warning_options_dep = declare_dependency(
    compile_args: cxx.get_supported_arguments(
      '-Wno-inline',
      '-Wno-attributes',
      # crow
      '-Wno-error=type-limits',
      # inside fmt (fmtlib/fmt#3354)
      '-Wno-error=array-bounds',
      '-Wno-error=stringop-overflow',
      # (fmtlib/fmt#3415)
      '-Wno-error=dangling-reference',
      # inside opencv
      '-Wno-error=deprecated-enum-enum-conversion',
      # inside msgpack
      '-Wno-error=maybe-uninitialized',
    )
  )
elif cxx.get_id() == 'clang' and cxx.get_argument_syntax() != 'msvc'
  warning_options_dep = declare_dependency(compile_args: [
    '-Wno-inline',
    '--system-header-prefix=msgpack.hpp',
    '--system-header-prefix=fmt/',
    '--system-header-prefix=spdlog/',
    '--system-header-prefix=curl/',
    '--system-header-prefix=crow.h',
    '--system-header-prefix=vips/',
    '--system-header-prefix=opencv2/',
    '--system-header-prefix=utf8.h',
    '--system-header-prefix=CLI/',
  ])
elif cxx.get_argument_syntax() == 'msvc'
  warning_options_dep = declare_dependency(compile_args: [
    '/wd4251',
    '/wd4275',
    '/wd4661',
    '/wd4706', # external:anglebracketしているのにvipsのヘッダーから出る
    '/external:anglebrackets',
    '/external:W0',
    '/utf-8',
    '/D_CRT_SECURE_NO_WARNINGS',
  ])
else
  warning('Unsupported compiler id \'' + cxx.get_id() + '\', ' + \
    'compiler argument syntax \'' + cxx.get_argument_syntax() + '\'',
  )
  warning_options_dep = []
endif
wabi_options_dep = declare_dependency(
  compile_args: cxx.get_supported_arguments(
    # https://gcc.gnu.org/onlinedocs/gcc/C_002b_002b-Dialect-Options.html
    '-fabi-version=11',
    '-Wabi',
    '-Wpsabi',
  ),
)

conf_data.set10('WEBCFACE_SYSTEM_VISIBILITY', webcface_system_visibility)
conf_data.set10('WEBCFACE_SYSTEM_DLLEXPORT', webcface_system_dllexport)
conf_data.set10('WEBCFACE_SYSTEM_WCHAR_WINDOWS', webcface_system_wchar_windows)
conf_data.set10('WEBCFACE_SYSTEM_WIN32SOCKET', webcface_system_win32socket)
conf_data.set10('WEBCFACE_SYSTEM_PATH_WINDOWS', webcface_system_path_windows)
conf_data.set10('WEBCFACE_SYSTEM_ADD_DEBUG', webcface_system_add_debug)

filesystem_proj = subproject('cpp-filesystem')
filesystem_dep = filesystem_proj.get_variable('cpp_filesystem_dep')
conf_data.set10('WEBCFACE_EXP_FILESYSTEM', filesystem_proj.get_variable('experimental'))

m_dep = cxx.find_library('m', required: false)
summary('libm', m_dep.found(), bool_yn: true)

if webcface_system_path_windows
  win32folders_dep = cxx.find_library('uuid')
else
  win32folders_dep = []
endif

if webcface_system_win32socket
  win32socket_deps = [
    cxx.find_library('iphlpapi'),
    cxx.find_library('ws2_32'),
    cxx.find_library('wsock32'), # asioの依存ライブラリなのでasioに追加されているべきでは?
  ]
else
  win32socket_deps = []
endif

configure_file(
  input: 'scripts' / 'config.h.in',
  output: 'webcface-config.h',
  configuration: conf_data,
  install: true,
  install_dir: get_option('includedir') / 'webcface' / 'common',
)
webcface_config_inc = include_directories('.')
webcface_config_args = [
  '-DWEBCFACE_MESON',
]
if get_option('unity') == 'on' or (get_option('unity') == 'subprojects' and meson.is_subproject())
  webcface_config_args += [
    '-DWEBCFACE_UNITY',
    '-D_USE_MATH_DEFINES',
  ]
endif
webcface_config_dep = declare_dependency(
  include_directories: [webcface_config_inc],
  compile_args: webcface_config_args,
)
version_rc_in = meson.project_source_root() / 'scripts' / 'version.rc.in'

if get_option('download_webui').allowed()
  download_webui = run_command(py,
    meson.project_source_root() / 'scripts' / 'fetch_webui.py',
    webcface_webui_version,
    meson.current_build_dir(),
    check: get_option('download_webui').enabled(),
  )
  if download_webui.stderr() != ''
    warning(download_webui.stderr())
  endif
  summary('WebUI',
    [download_webui.returncode() == 0, webcface_webui_version],
    bool_yn: true,
    list_sep: ' ',
  )
  if download_webui.returncode() == 0
    install_subdir(meson.current_build_dir() / 'dist',
      install_dir: get_option('datadir') / 'webcface',
    )
  endif
else
  summary('WebUI', false, bool_yn: true)
endif

build_server = get_option('server').enabled() \
  or (get_option('server').allowed() \
    and not meson.is_subproject() \
    and host_machine.system() != 'cygwin' \
  )
build_examples = get_option('examples').enabled() \
  or (get_option('examples').allowed() and not meson.is_subproject())
build_cv_examples = get_option('cv_examples').enabled() \
  or (get_option('cv_examples').allowed() and not meson.is_subproject())
build_tests = get_option('tests').enabled() \
  or (get_option('tests').allowed() and not meson.is_subproject())

msgpack_cxx_dep = dependency('msgpack-cxx')
fmt_dep = dependency('fmt', version: '>=11')
if fmt_dep.type_name() == 'internal'
  spdlog_dep = subproject('spdlog').get_variable('spdlog_dep')
else
  spdlog_dep = dependency('spdlog')
endif
utf8cpp_dep = dependency('utf8cpp')

# curlはvisibilityがhiddenになっている
curl_dep = dependency('libcurl', version: '>=7.86.0')
if curl_dep.type_name() != 'internal'
  if curl_dep.version().version_compare('>=8.10.0') and \
      curl_dep.version().version_compare('<8.12.0')
    # https://github.com/curl/curl/issues/15865
    error('The WebSocket support in libcurl versions 8.10.0 through 8.12.0 has a bug. ' +
      'Use other versions or set "--force-fallback-for=libcurl".',
    )
  endif
  curl_dep_version = curl_dep.version().split('-')[0].split('.')
  curl_runtime_version_test = cxx.run('''
    #include <curl/curl.h>
    #include <cassert>
    #include <iostream>
    int main() {
      auto info = curl_version_info(CURLVERSION_NOW);
      std::cerr << "version_num = " << std::hex << info->version_num << std::endl;
      assert(((info->version_num >> 16) & 0xff) == ''' + curl_dep_version[0] + ''');
      assert(((info->version_num >> 8) & 0xff) == ''' + curl_dep_version[1] + ''');
      assert(((info->version_num >> 0) & 0xff) == ''' + curl_dep_version[2] + ''');
    }
    ''',
    dependencies: [curl_dep],
    name: 'curl_runtime_version_test',
  )
  assert(curl_runtime_version_test.compiled())
  if curl_runtime_version_test.returncode() != 0
    error('Could not run test using libcurl. ' + \
      'Check the install_name, PATH, LD_LIBRARY_PATH or DYLD_LIBRARY_PATH, ' + \
      'or if you are unsure set "--force-fallback-for=libcurl" to build libcurl from source.',
    )
  endif
  curl_ws_support_test = cxx.run('''
    #include <curl/curl.h>
    #include <cassert>
    #include <cstddef>
    int main() {
        CURL *curl = curl_easy_init();
        std::size_t s;
        assert(curl_ws_send(curl, nullptr, 0, &s, 0, 0) != CURLE_NOT_BUILT_IN);
    }
    ''',
    dependencies: [curl_dep],
    name: 'curl_ws_support_test',
  )
  assert(curl_ws_support_test.compiled())
  if curl_ws_support_test.returncode() != 0
    error('The installed curl has no websocket support, so WebCFace cannot use it. ' + \
      'Set "--force-fallback-for=libcurl".',
    )
  endif
endif

if build_server
  cli11_dep = dependency('cli11', 'CLI11')

  crow_dep = dependency('Crow')
  if crow_dep.type_name() != 'internal'
    crow_local_socket_support_test = cxx.compiles('''
      #include <crow.h>
      int main(){
          crow::SimpleApp app;
          CROW_ROUTE(app, "/")([](){
              return "Hello world";
          });
          app.local_socket_path("/tmp/test.sock").multithreaded().run();
      }
      ''',
      dependencies: [crow_dep],
      name: 'crow_local_socket_support_test',
    )
    if not crow_local_socket_support_test
      error('The installed Crow has no local socket support, so WebCFace cannot use it. ' + \
        'Set "--force-fallback-for=Crow".',
      )
    endif
  endif

  libvips_cpp_dep = dependency('vips-cpp')
else
  libvips_cpp_dep = disabler()
endif

if build_examples and build_cv_examples
  opencv_core_dep = dependency('OpenCV',
    method: 'cmake',
    modules: ['opencv_core'],
    required: false,
    disabler: true,
  )
  opencv_imgproc_dep = dependency('OpenCV',
    method: 'cmake',
    modules: ['opencv_imgproc'],
    required: false,
    disabler: true,
  )
  opencv_highgui_dep = dependency('OpenCV',
    method: 'cmake',
    modules: ['opencv_highgui'],
    required: false,
    disabler: true,
  )
else
  opencv_core_dep = disabler()
  opencv_imgproc_dep = disabler()
  opencv_highgui_dep = disabler()
endif

if build_tests
  gtest_dep = dependency('gtest', main: true,
    fallback: ['gtest', 'gtest_main_dep'],
  )
  if build_server
    benchmark_dep = dependency('benchmark')
  endif
endif

summary('Build server',
  [build_server, get_option('server').auto() ? '(auto)' : ''],
  bool_yn: true,
  list_sep: ' ',
)
summary('Build examples',
  [build_examples, get_option('examples').auto() ? '(auto)' : ''],
  bool_yn: true,
  list_sep: ' ',
)
summary('Build examples with OpenCV',
  [
    build_examples and \
      build_cv_examples and \
      not is_disabler(opencv_core_dep) and \
      not is_disabler(opencv_imgproc_dep) and \
      not is_disabler(opencv_highgui_dep),
    get_option('cv_examples').auto() ? '(auto)' : '',
  ],
  bool_yn: true,
  list_sep: ' ',
)
summary('Build tests',
  [build_tests, get_option('tests').auto() ? '(auto)' : ''],
  bool_yn: true,
  list_sep: ' ',
)

subdir('common')
subdir('client')

sources = []
if webcface_system_version_rc
  sources += import('windows').compile_resources(
    configure_file(
      input: version_rc_in,
      output: 'webcface_version.rc',
      configuration: configuration_data({
        'company_name': webcface_contact,
        'file_description': 'WebCFace Client Library',
        'filetype_app': 0,
      }),
    ),
    include_directories: [webcface_config_inc],
  )
endif

webcface_lib = library(webcface_lib_name,
  # link_wholeではなくobjectsにすることで、
  # それぞれのlibに追加したcpp_argsとかが含まれないようにする
  objects: [
    webcface_common_lib.extract_all_objects(recursive: false),
    webcface_client_lib.extract_all_objects(recursive: false),
  ],
  sources: sources,
  dependencies: [
    filesystem_dep,
    m_dep,
    win32folders_dep,
    msgpack_cxx_dep,
    utf8cpp_dep,
    spdlog_dep,
    curl_dep,
  ],
  version: meson.project_version(),
  soversion: webcface_abi_major,
  darwin_versions: [
    webcface_abi_major + '.' + webcface_abi_minor,
    meson.project_version(),
  ],
  install: true,
)
webcface_dep = declare_dependency(
  link_with: [webcface_lib],
  include_directories: [
    webcface_common_inc,
    webcface_client_inc,
  ],
  dependencies: [
    webcface_config_dep,
    filesystem_dep,
    win32folders_dep,
  ],
)
import('pkgconfig').generate(
  name: 'WebCFace',
  description: webcface_description,
  url: webcface_url,
  filebase: 'webcface',
  libraries: [webcface_lib],
  version: webcface_version_str,
  # インストール後に webcface.pc へのシンボリックリンクリンクを別の場所に貼った場合、
  # もとのprefix (/opt/webcface など) も参照するようにする。
  # (cmakeの場合はwebcface-config.cmake内にシンボリックリンクを展開するコード書いた)
  extra_cflags: '-I' + (get_option('prefix') / get_option('includedir')),
)
meson.override_dependency('webcface', webcface_dep)
if get_option('default_library') == 'shared'
  # sharedライブラリの場合のみ、webcface.pc から Requires.private の行を削除
  meson.add_install_script(py,
    meson.project_source_root() / 'scripts' / 'remove_private_requires.py',
  )
endif

if build_server
  subdir('server-store')
  subdir('server')
endif
if build_examples
  subdir('examples')
endif
if build_tests
  subdir('tests')
  if build_server
    subdir('benchmarks')
  endif
endif

headers = run_command(py,
  meson.project_source_root() / 'scripts' / 'list_headers.py',
  check: true,
).stdout().strip().split('\n')
foreach h: headers
  h_root = h.split(';')[0]
  h_dir = h.split(';')[1]
  h_name = h.split(';')[2]
  install_headers(h_root / h_dir / h_name,
    subdir: h_dir,
  )
endforeach
install_subdir('client/include/webcface/modules', install_dir: 'include/webcface')

if host_machine.system() == 'linux'
  conf_data = configuration_data({
    'prefix': get_option('prefix'),
    'bindir': get_option('bindir'),
  })
  configure_file(
    input: 'scripts' / 'webcface-server.service.in',
    output: 'webcface-server.service',
    configuration: conf_data,
    install: true,
    install_dir: 'lib' / 'systemd' / 'system', # lib/x86_64-linux-gnu/ ではない
  )
endif
install_data(['README.md', 'LICENSE'],
  install_dir: get_option('datadir') / 'doc' / 'webcface',
)
if get_option('default_library') == 'shared'
  cmake.write_basic_package_version_file(
    name: 'webcface',
    version: meson.project_version(),
    compatibility: 'SameMajorVersion',
    arch_independent: false,
    install_dir: get_option('libdir') / 'cmake' / 'webcface' + meson.project_version().split('.')[0],
  )
  conf_data = configuration_data({
    'prefix': get_option('prefix'),
    'libdir': get_option('libdir'),
    'includedir': get_option('includedir'),
    'webcface_lib_debug': webcface_lib_name_debug,
    'webcface_lib_release': webcface_lib_name_release,
    'webcface_abi_major': webcface_abi_major,
    'webcface_version_str': webcface_version_str,
  })
  cmake.configure_package_config_file(
    name: 'webcface',
    input: 'scripts' / 'webcface-config.cmake.in',
    configuration: conf_data,
    install_dir: get_option('libdir') / 'cmake' / 'webcface' + meson.project_version().split('.')[0],
  )
endif

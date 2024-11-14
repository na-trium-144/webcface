project('webcface', 'c', 'cpp',
  version: '2.4.2',
  license: 'MIT',
  meson_version: '>=1.3.0',
  default_options: [
    'buildtype=release',
    'warning_level=3', # everythingは現在たぶん通らない
    'werror=true',
    # 'c_std=c99', # 指定するとlibjpegのビルドが通らない
    'cpp_std=gnu++17,c++17', # spdlogがgnu++必要、webcfaceは17以上必須
    'wrap_mode=forcefallback',
  ],
)
fs = import('fs')
cmake = import('cmake')
cxx = meson.get_compiler('cpp')
py = find_program('python3')

webcface_abi_major = '21' # ABIの破壊的変更で1増やす
webcface_abi_minor = '0' # ABIの追加で1増やす
webcface_webui_version = '1.10.0'

webcface_description = 'Web-based IPC & Dashboard-like UI'
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
magick_vs = false
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
    magick_vs = true
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
  warning_options = declare_dependency(
    compile_args: cxx.get_supported_arguments(
      '-Wno-inline',
      '-Wno-error=psabi', # refer to PR#9
      # crow
      '-Wno-error=type-limits',
      # inside fmt (fmtlib/fmt#3354)
      '-Wno-error=array-bounds',
      '-Wno-error=stringop-overflow',
      # (fmtlib/fmt#3415)
      '-Wno-error=dangling-reference',
      # inside opencv
      '-Wno-error=deprecated-enum-enum-conversion',
      # inside magick
      '-Wno-error=ignored-qualifiers',
    )
  )
elif cxx.get_id() == 'clang' and cxx.get_argument_syntax() != 'msvc'
  warning_options = declare_dependency(compile_args: [
    '-Wno-inline',
    '--system-header-prefix=msgpack.hpp',
    '--system-header-prefix=spdlog/',
    '--system-header-prefix=curl/',
    '--system-header-prefix=crow.h',
    '--system-header-prefix=Magick++.h',
    '--system-header-prefix=opencv2/',
    '--system-header-prefix=utf8.h',
    '--system-header-prefix=CLI/',
  ])
elif cxx.get_argument_syntax() == 'msvc'
  warning_options = declare_dependency(compile_args: [
    '/wd4251',
    '/wd4275',
    '/wd4661',
    '/external:anglebrackets',
    '/external:W0',
    '/utf-8',
    '/D_CRT_SECURE_NO_WARNINGS',
  ])
else
  warning('Unsupported compiler id \'' + cxx.get_id() + '\', ' + \
    'compiler argument syntax \'' + cxx.get_argument_syntax() + '\'',
  )
  warning_options = []
endif

conf_data.set10('WEBCFACE_SYSTEM_VISIBILITY', webcface_system_visibility)
conf_data.set10('WEBCFACE_SYSTEM_DLLEXPORT', webcface_system_dllexport)
conf_data.set10('WEBCFACE_SYSTEM_WCHAR_WINDOWS', webcface_system_wchar_windows)
conf_data.set10('WEBCFACE_SYSTEM_WIN32SOCKET', webcface_system_win32socket)
conf_data.set10('WEBCFACE_SYSTEM_PATH_WINDOWS', webcface_system_path_windows)
conf_data.set10('WEBCFACE_SYSTEM_ADD_DEBUG', webcface_system_add_debug)

filesystem_dep = disabler()
webcface_exp_filesystem = false
filesystem_check_src = '''
#if defined(CXX17_FILESYSTEM)
#include <filesystem>
namespace std_fs = std::filesystem;
#elif defined(CXX11_EXP_FILESYSTEM)
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#endif
int main(void) {
    return std_fs::exists(std_fs::path("/")), 0;
}
'''
if not meson.can_run_host_binaries()
  cxx17_filesystem_compiles = cxx.compiles(filesystem_check_src,
    args: ['-DCXX17_FILESYSTEM'],
    name: 'cxx17_filesystem',
  )
  if cxx17_filesystem_compiles
    filesystem_dep = declare_dependency()
    summary('filesystem', 'C++17')
    summary('libstdc++fs', 'skipped while cross compiling')
  endif
else
  cxx17_filesystem_ret = cxx.run(filesystem_check_src,
    args: ['-DCXX17_FILESYSTEM'],
    name: 'cxx17_filesystem',
  )
  if cxx17_filesystem_ret.compiled() and cxx17_filesystem_ret.returncode() == 0
    filesystem_dep = declare_dependency()
    summary('filesystem', 'C++17')
    summary('libstdc++fs', false, bool_yn: true)
  else
    libfs = cxx.find_library('stdc++fs', required: false)
    if libfs.found()
      cxx17_filesystem_libfs_ret = cxx.run(filesystem_check_src,
        args: ['-DCXX17_FILESYSTEM'],
        name: 'cxx17_filesystem_libfs',
        dependencies: [libfs],
      )
      if cxx17_filesystem_libfs_ret.compiled() and cxx17_filesystem_libfs_ret.returncode() == 0
        filesystem_dep = libfs
        summary('filesystem', 'C++17')
        summary('libstdc++fs', true, bool_yn: true)
      endif
    endif
  endif
endif
if is_disabler(filesystem_dep)
  if not meson.can_run_host_binaries()
    cxx11_exp_filesystem_compiles = cxx.compiles(filesystem_check_src,
      args: ['-DCXX11_EXP_FILESYSTEM'],
      name: 'cxx11_exp_filesystem',
    )
    if cxx11_exp_filesystem_compiles
      filesystem_dep = declare_dependency()
      webcface_exp_filesystem = true
      summary('filesystem', 'experimental')
      summary('libstdc++fs', 'skipped while cross compiling')
    endif
  else
    cxx11_exp_filesystem_ret = cxx.run(filesystem_check_src,
      args: ['-DCXX11_EXP_FILESYSTEM'],
      name: 'cxx11_exp_filesystem',
    )
    if cxx11_exp_filesystem_ret.compiled() and cxx11_exp_filesystem_ret.returncode() == 0
      filesystem_dep = declare_dependency()
      webcface_exp_filesystem = true
      summary('filesystem', 'experimental')
      summary('libstdc++fs', false, bool_yn: true)
    else
      libfs = cxx.find_library('stdc++fs', required: false)
      if libfs.found()
        cxx11_exp_filesystem_libfs_ret = cxx.run(filesystem_check_src,
          args: ['-DCXX11_EXP_FILESYSTEM'],
          name: 'cxx11_exp_filesystem_libfs',
          dependencies: [libfs],
        )
        if cxx11_exp_filesystem_libfs_ret.compiled() and cxx11_exp_filesystem_libfs_ret.returncode() == 0
          filesystem_dep = libfs
          webcface_exp_filesystem = true
          summary('filesystem', 'experimental')
          summary('libstdc++fs', true, bool_yn: true)
        endif
      endif
    endif
  endif
endif
if is_disabler(filesystem_dep)
  error('Neither std::filesystem or std::experimental::filesystem available')
endif
conf_data.set10('WEBCFACE_EXP_FILESYSTEM', webcface_exp_filesystem)

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
webcface_config_dep = declare_dependency(
  include_directories: [webcface_config_inc],
  compile_args: [
    '-DWEBCFACE_MESON',
  ],
)
version_rc_in = meson.project_source_root() / 'scripts' / 'version.rc.in'

if get_option('download_webui').allowed()
  download_webui = run_command(py,
    meson.project_source_root() / 'scripts' / 'fetch_webui.py',
    webcface_webui_version,
    meson.current_build_dir(),
    check: get_option('download_webui').enabled(),
  )
  if download_webui.returncode() == 0
    message(download_webui.stdout())
    summary('WebUI',
      [true, webcface_webui_version, 'Downloaded to ' + (meson.current_build_dir() / 'dist')],
      bool_yn: true,
      list_sep: ' ',
    )
    install_subdir(meson.current_build_dir() / 'dist',
      install_dir: get_option('datadir') / 'webcface',
    )
  else
    warning(download_webui.stderr())
    summary('WebUI',
      [false, webcface_webui_version, 'Failed to download'],
      bool_yn: true,
      list_sep: ' ',
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

msgpack_cxx_dep = dependency('msgpack-cxx',
  default_options: [
    'boost=disabled',
    'tests=disabled',
    'std_variant=false', # msvcでなんかエラーになる
  ],
)
spdlog_dep = dependency('spdlog',
  default_options: [
    'tests=disabled',
    'compile_library=false',
    'external_fmt=disabled',
    'std_format=disabled',
  ],
)
utf8cpp_dep = dependency('utf8cpp')

if get_option('wrap_mode') != 'forcefallback' and not get_option('force_fallback_for').contains('libcurl')
  curl_dep = dependency('libcurl', required: false)
else
  curl_dep = dependency('', required: false)
endif
if curl_dep.found()
  curl_has_ws_support = cxx.run('''
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
    name: 'curl_has_ws_support',
  )
  if not curl_has_ws_support.compiled() or curl_has_ws_support.returncode() != 0
    error('The installed curl has no websocket support, so WebCFace cannot use it.\n' + \
      'Set "-Dforce_fallback_for=[\'libcurl\']".',
    )
  endif
else
  curl_dep = subproject('curl',
    default_options: [
      'warning_level=0',
      'werror=false',
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
      'http2=disabled',
      'ipv6=disabled',
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
  ).get_variable('curl_dep')
endif

if build_server
  cli11_dep = dependency('cli11', 'CLI11')

  if get_option('wrap_mode') != 'forcefallback' and not get_option('force_fallback_for').contains('Crow')
    crow_dep = dependency('Crow', required: false)
  else
    crow_dep = dependency('', required: false)
  endif
  if crow_dep.found()
    crow_has_unix_socket_support = cxx.compiles('''
      #include <crow.h>
      int main(){
          crow::SimpleApp app;
          CROW_ROUTE(app, "/")([](){
              return "Hello world";
          });
          app.unix_path("/tmp/test.sock").multithreaded().run();
      }
      ''',
      dependencies: [crow_dep],
      name: 'crow_has_unix_socket_support',
    )
    if not crow_has_unix_socket_support
      error('The installed Crow has no unix socket support, so WebCFace cannot use it.\n' + \
        'Set "-Dforce_fallback_for=[\'Crow\']".',
      )
    endif
  else
    crow_dep = subproject('crow').get_variable('crow_dep')
  endif

  if magick_vs
    if debug_crt
      magick_libtype = 'DB'
    else
      magick_libtype = 'RL'
    endif
    magickpp_dep = dependency('', required: false)

    if get_option('wrap_mode') != 'forcefallback' and not get_option('force_fallback_for').contains('Magick++')
      magick = find_program('magick',
        required: get_option('wrap_mode') == 'nofallback',
      )
      if magick.found()
        find_magick = run_command(py,
          meson.project_source_root() / 'scripts' / 'magick_msvc_find.py',
          fs.parent(magick.full_path()),
          magick_libtype,
          check: true,
        ).stdout().strip().split('\n')
        if find_magick[0] == '' or find_magick[1] == '' or find_magick.length() < 3
          warning('ImageMagick found but its ' + \
            (debug_crt ? 'debug' : 'release') + \
            ' libraries and headers was not found.',
          )
        else
          magick_ver = run_command('magick', '-version', check: false)
          if magick_ver.returncode() != 0
            error('Failed to get version information from installed ImageMagick.\n' + \
              'Set "-Dforce_fallback_for=[\'Magick++\']".\n\n' + \
              'output:\n' + \
              magick_ver.stderr(),
            )
          elif magick_ver.stdout().contains('OpenMP')
            error('The installed ImageMagick uses openmp, so WebCFace cannot use it.\n' + \
              'Set "-Dforce_fallback_for=[\'Magick++\']".',
            )
          endif

          magick_libs = []
          foreach lib: find_magick
            if lib.endswith('_.lib')
              magick_libs += cxx.find_library(
                lib.strip('.lib'),
                dirs: [find_magick[1]],
              )
            endif
          endforeach
          magickpp_dep = declare_dependency(
            include_directories: include_directories(find_magick[0]),
            dependencies: magick_libs,
          )
        endif
      endif
    endif

    if not magickpp_dep.found()
      message('running magick_msvc_configure.py...')
      magick_script_args = [
        meson.project_source_root(),
        meson.current_build_dir(),
        magick_libtype,
        cxx.version(),
        host_machine.cpu_family(),
      ]
      config_magick = run_command(py,
        meson.project_source_root() / 'scripts' / 'magick_msvc_configure.py',
        magick_script_args,
        check: false,
      )
      if config_magick.returncode() != 0
        error('Failed to configure ImageMagick-Windows\n\n' + \
          'stdout:\n' + \
          config_magick.stdout() + \
          '\nstderr:\n' + \
          config_magick.stderr(),
        )
      endif
      find_magick = run_command(py,
        meson.project_source_root() / 'scripts' / 'magick_msvc_dep.py',
        magick_script_args,
        check: true,
      ).stdout().strip().split('\n')
      magick_libs = []
      foreach lib: find_magick
        if lib.endswith('_.lib')
          # must not contain a path segment.
          magick_libs += lib
        endif
      endforeach
      magickpp_dep = declare_dependency(
        compile_args: ['-DSTATIC_MAGICK'],
        include_directories: include_directories(find_magick[0].split(',')),
        sources: custom_target('imagemagick-windows',
          build_always_stale: true,
          command: [py,
            meson.project_source_root() / 'scripts' / 'magick_msvc_build.py',
            magick_script_args,
          ],
          console: true,
          output: magick_libs,
        ),
      )
    endif

  else
    if get_option('wrap_mode') != 'forcefallback' and not get_option('force_fallback_for').contains('Magick++')
      magickpp_dep = dependency('Magick++', method: 'pkg-config', required: false)
    else
      magickpp_dep = dependency('', required: false)
    endif
    if magickpp_dep.found()
      pkg_config = find_program('pkg-config')
      magickcore_cflags = run_command(
        pkg_config, '--cflags', 'MagickCore',
        env: {
          'PKG_CONFIG_PATH': get_option('pkg_config_path'),
        },
        check: true,
      )
      if magickcore_cflags.stdout().contains('fopenmp')
        error('The installed ImageMagick uses openmp, so WebCFace cannot use it.\n' + \
          'Set "-Dforce_fallback_for=[\'Magick++\']".',
        )
      endif
    else
      dependency('libjpeg',
        default_options: [
          'default_library=static',
          'jpeg-turbo=disabled',
          'tests=disabled',
          'warning_level=0',
          'werror=false',
        ]
      )
      dependency('zlib',
        default_options: [
          'default_library=static',
          'tests=disabled',
          'warning_level=0',
          'werror=false',
        ]
      )
      dependency('libpng',
        default_options: [
          'default_library=static',
          'warning_level=0',
          'werror=false',
        ]
      )
      dependency('libwebp',
        default_options: [
          'default_library=static',
          'cwebp=disabled',
          'dwebp=disabled',
          'webpinfo=disabled',
          'webpmux=disabled',
          'warning_level=0',
          'werror=false',
        ]
      )
      magickpp_dep = subproject('imagemagick').get_variable('magickpp_dep')
    endif
  endif
  # if magickpp_dep.version() == 'unknown' or magickpp_dep.version() == 'undefined' or magickpp_dep.version().version_compare('>=7.0.0')
  # msvcでなぜか version() = '2.0.0' が返るので一旦チェックを無効にしている
  # (どうせver6以下でビルドすることはほぼない)
  magickpp_dep = declare_dependency(
    compile_args: ['-DWEBCFACE_MAGICK_VER7'],
    dependencies: [magickpp_dep],
  )
  # endif
else
  magickpp_dep = disabler()
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

# 外部ライブラリのシンボルを公開しないようにするために
# 以前は -Wl,--exclude-libs や -Wl,-hidden-l オプションが必要だったが、
# spdlogはヘッダーオンリーにし、
# curlはvisibilityがhiddenになっており、
# magickppとasioはserverのみにしかリンクしないようにしたので
# 特殊なリンクオプションは一切不要
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

if host_machine.system() == 'linux'
  install_data('scripts' / 'webcface-server.service',
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

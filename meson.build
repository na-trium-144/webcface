project('webcface', 'c', 'cpp',
  version: '2.0.0',
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

webcface_soversion = '20'
webcface_webui_version = '1.7.0'

webcface_description = 'Web-based IPC & Dashboard-like UI'
webcface_contact = 'na-trium-144@users.noreply.github.com'

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
summary('ABI Version', webcface_soversion)

assert(
  get_option('default_library') == 'shared' or get_option('default_library') == 'static',
  'building both shared and static webcface libraries is currently not supported',
)
summary('Library', get_option('default_library'))
summary('Build type', get_option('buildtype'))
conf_data = configuration_data({
  'WEBCFACE_VERSION_MAJOR': meson.project_version().split('.')[0],
  'WEBCFACE_VERSION_MINOR': meson.project_version().split('.')[1],
  'WEBCFACE_VERSION_REVISION': meson.project_version().split('.')[2],
  'WEBCFACE_VERSION': '"@webcface_version_str@"',
  'PROJECT_DESCRIPTION': webcface_description,
  'WEBCFACE_CONTACT': webcface_contact,
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
magick_vs = false
if host_machine.system() == 'linux'
  summary('System', 'linux')
  webcface_system_visibility = true
  webcface_system_exclude_libs = true
elif host_machine.system() == 'darwin'
  summary('System', 'darwin')
  webcface_system_visibility = true
  webcface_system_hidden_l = true
elif host_machine.system() == 'windows'
  webcface_system_dllexport = true
  webcface_system_wchar_windows = true
  webcface_system_win32socket = true
  webcface_system_path_windows = true
  if cxx.get_argument_syntax() == 'msvc'
    summary('System', 'windows (msvc)')
    webcface_system_add_debug = true
    magick_vs = true
  else
    summary('System', 'windows (mingw)')
  endif
elif host_machine.system() == 'cygwin'
  summary('System', 'windows (cygwin)')
  webcface_system_dllexport = true
  webcface_system_wchar_windows = true
else
  warning('unsupported system: ' + host_machine.system())
endif
summary('Platform', host_machine.cpu_family())

if (cxx.get_id() == 'gcc' or cxx.get_id() == 'clang') and cxx.get_argument_syntax() != 'msvc'
  warning_options = declare_dependency(compile_args: [
    '-Wno-inline',
  ])
elif cxx.get_argument_syntax() == 'msvc'
  warning_options = declare_dependency(compile_args: [
    '/wd4251',
    '/wd4275',
    '/wd4661',
    '/wd4996',
    '/external:anglebrackets',
    '/external:W0',
    '/utf-8',
  ])
else
  warning('Unsupported compiler id \'' + cxx.get_id() + '\', ' + \
    'compiler argument syntax \'' + cxx.get_argument_syntax() + '\'',
  )
  warning_options = declare_dependency()
endif

conf_data.set10('WEBCFACE_SYSTEM_VISIBILITY', webcface_system_visibility)
conf_data.set10('WEBCFACE_SYSTEM_DLLEXPORT', webcface_system_dllexport)
conf_data.set10('WEBCFACE_SYSTEM_WCHAR_WINDOWS', webcface_system_wchar_windows)
conf_data.set10('WEBCFACE_SYSTEM_WIN32SOCKET', webcface_system_win32socket)
conf_data.set10('WEBCFACE_SYSTEM_PATH_WINDOWS', webcface_system_path_windows)
conf_data.set10('WEBCFACE_SYSTEM_ADD_DEBUG', webcface_system_add_debug)

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
libfs = cxx.find_library('stdc++fs', required: false)
cxx17_filesystem_ret = cxx.run(filesystem_check_src,
  args: ['-DCXX17_FILESYSTEM'],
  name: 'cxx17_filesystem'
)
if cxx17_filesystem_ret.compiled() and cxx17_filesystem_ret.returncode() == 0
  filesystem_dep = declare_dependency()
else
  if libfs.found()
    cxx17_filesystem_libfs_ret = cxx.run(filesystem_check_src,
      args: ['-DCXX17_FILESYSTEM'],
      dependencies: [libfs],
      name: 'cxx17_filesystem_libfs'
    )
  endif
  if libfs.found() and cxx17_filesystem_libfs_ret.compiled() and cxx17_filesystem_libfs_ret.returncode() == 0
    filesystem_dep = libfs
  else
    cxx11_exp_filesystem_ret = cxx.run(filesystem_check_src,
      args: ['-DCXX11_EXP_FILESYSTEM'],
      name: 'cxx11_exp_filesystem'
    )
    if cxx11_exp_filesystem_ret.compiled() and cxx11_exp_filesystem_ret.returncode() == 0
      filesystem_dep = declare_dependency()
      webcface_exp_filesystem = true
    else
      if libfs.found()
        cxx11_exp_filesystem_libfs_ret = cxx.run(filesystem_check_src,
          args: ['-DCXX11_EXP_FILESYSTEM'],
          dependencies: [libfs],
          name: 'cxx11_exp_filesystem_libfs'
        )
      endif
      if libfs.found() and cxx11_exp_filesystem_libfs_ret.compiled() and cxx11_exp_filesystem_libfs_ret.returncode() == 0
        filesystem_dep = libfs
        webcface_exp_filesystem = true
      else
        error('std::filesystem not available')
      endif
    endif
  endif
endif
conf_data.set10('WEBCFACE_EXP_FILESYSTEM', webcface_exp_filesystem)

configure_file(
  input: 'config.h.in',
  output: 'webcface-config.h',
  configuration: conf_data
)
configure_file(
  input: 'version.rc.in',
  output: 'version.rc',
  configuration: conf_data
)
webcface_config_dep = declare_dependency(
  include_directories: [include_directories('.')],
  compile_args: [
    '-DWEBCFACE_MESON',
  ],
)

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
summary('Build server',
  [build_server, get_option('server').auto().to_string('(auto)', '')],
  bool_yn: true,
  list_sep: ' ',
)
build_examples = get_option('examples').enabled() \
  or (get_option('examples').allowed() and not meson.is_subproject())
summary('Build examples',
  [build_examples, get_option('examples').auto().to_string('(auto)', '')],
  bool_yn: true,
  list_sep: ' ',
)
build_tests = get_option('tests').enabled() \
  or (get_option('tests').allowed() and not meson.is_subproject())
summary('Build tests',
  [build_tests, get_option('tests').auto().to_string('(auto)', '')],
  bool_yn: true,
  list_sep: ' ',
)

msgpack_cxx_dep = dependency('msgpack-cxx',
  default_options: [
    'boost=disabled',
    'tests=disabled',
    'std_variant=false', # msvcでなんかエラーになる
  ],
).as_system('system')
spdlog_dep = dependency('spdlog',
  default_options: [
    'tests=disabled',
    'compile_library=false',
    'external_fmt=disabled',
    'std_format=disabled',
  ],
).as_system('system')
utf8cpp_dep = dependency('utf8cpp').as_system('system')

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
    error('''The installed curl has no websocket support, so WebCFace cannot use it.
Set "-Dforce_fallback_for=['libcurl']".''')
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
curl_dep = curl_dep.as_system('system')

if build_server
  crow_dep = dependency('Crow').as_system('system')
  cli11_dep = dependency('cli11').as_system('system')

  if magick_vs
    if get_option('buildtype') == 'debug'
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
            (get_option('buildtype') == 'debug' ? 'debug' : 'release') + \
            ' libraries and headers was not found.',
          )
        else
          magick_ver = run_command('magick', '-version', check: false)
          if magick_ver.returncode() != 0
            error('''Failed to get version information from installed ImageMagick.
Set "-Dforce_fallback_for=['Magick++']".''' + '\n\noutput:\n' + \
              magick_ver.stderr(),
            )
          elif magick_ver.stdout().contains('OpenMP')
            error('''The installed ImageMagick uses openmp, so WebCFace cannot use it.
Set "-Dforce_fallback_for=['Magick++']".''')
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
        error('Failed to configure ImageMagick-Windows\n\nstdout:\n' + \
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
      # todo: このチェックは機能してない
      magickpp_enables_openmp = cxx.compiles('''
int main() {
  #pragma omp parallel for
  for (int i = 0; i < 1000; i++) {
  }
}
''',
        dependencies: [magickpp_dep],
        name: 'magickpp_enables_openmp'
      )
      if magickpp_enables_openmp
        error('''The installed ImageMagick uses openmp, so WebCFace cannot use it.
Set "-Dforce_fallback_for=['Magick++']".''')
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
  magickpp_dep = magickpp_dep.as_system('system')
endif

if build_examples
  opencv_dep = dependency('OpenCV', required: false).as_system('system')
endif

if build_tests
  gtest_proj = subproject('gtest')
  gtest_dep = gtest_proj.get_variable('gtest_dep').as_system('system')
  gtest_main_dep = gtest_proj.get_variable('gtest_main_dep').as_system('system')
endif

subdir('encoding')
subdir('message')
subdir('client')

# 外部ライブラリのシンボルを公開しないようにするために
# 以前は -Wl,--exclude-libs や -Wl,-hidden-l オプションが必要だったが、
# spdlogはヘッダーオンリーにし、
# curlはvisibilityがhiddenになっており、
# magickppとasioはserverのみにしかリンクしないようにしたので
# 特殊なリンクオプションは一切不要
webcface_lib = library('webcface',
  link_whole: [
    webcface_encoding_lib,
    webcface_message_lib,
    webcface_client_lib,
  ],
  dependencies: [
    filesystem_dep,
    spdlog_dep,
    curl_dep,
  ],
  version: meson.project_version(),
  soversion: webcface_soversion,
)
webcface_dep = declare_dependency(
  link_with: [webcface_lib],
  include_directories: [
    webcface_encoding_inc,
    webcface_client_inc,
  ],
  dependencies: [
    webcface_config_dep,
    filesystem_dep,
  ],
)

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


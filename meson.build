project('webcface', 'c', 'cpp',
  version: '2.0.0',
  license: 'MIT',
  meson_version: '>=0.59.0',
  default_options: [
    'buildtype=release',
    'warning_level=3', # everythingはたぶん通らない
    'werror=true',
    'c_std=c99',
    'cpp_std=c++17',
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
summary('Version', webcface_version_str, section: 'Configuration')

assert(
  get_option('default_library') == 'shared' or get_option('default_library') == 'static',
  'building both shared and static webcface libraries is currently not supported',
)
summary('Library', get_option('default_library'), section: 'Configuration')
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
if host_machine.system() == 'linux'
  summary('System', 'linux', section: 'Configuration')
  webcface_system_visibility = true
  webcface_system_exclude_libs = true
elif host_machine.system() == 'darwin'
  summary('System', 'darwin', section: 'Configuration')
  webcface_system_visibility = true
  webcface_system_hidden_l = true
elif host_machine.system() == 'windows'
  webcface_system_dllexport = true
  webcface_system_wchar_windows = true
  webcface_system_win32socket = true
  webcface_system_path_windows = true
  if cxx.get_argument_syntax() == 'msvc'
    summary('System', 'windows (msvc)', section: 'Configuration')
    webcface_system_add_debug = true
  else
    summary('System', 'windows (mingw)', section: 'Configuration')
  endif
elif host_machine.system() == 'cygwin'
  summary('System', 'windows (cygwin)', section: 'Configuration')
  webcface_system_dllexport = true
  webcface_system_wchar_windows = true
else
  warning('unsupported system: ' + host_machine.system())
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
add_project_arguments(
  '-DWEBCFACE_MESON',
  language: ['c', 'cpp'],
)
webcface_config_inc = include_directories('.')

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
      webcface_webui_version + ' Downloaded to ' + (meson.current_build_dir() / 'dist'),
      section: 'Configuration',
    )
  else
    warning(download_webui.stderr())
    summary('WebUI',
      webcface_webui_version + ' Failed to download',
      section: 'Configuration',
    )
  endif
else
  summary('WebUI', false, section: 'Configuration')
endif

build_server = get_option('server').enabled() \
  or (get_option('server').allowed() \
    and not meson.is_subproject() \
    and host_machine.system() != 'cygwin' \
  )
summary('Build server',
  build_server.to_string() + get_option('server').auto().to_string(' (auto)', ''),
  section: 'Configuration',
)
build_examples = get_option('examples').enabled() \
  or (get_option('examples').allowed() and not meson.is_subproject())
summary('Build examples',
  build_examples.to_string() + get_option('examples').auto().to_string(' (auto)', ''),
  section: 'Configuration',
)
build_tests = get_option('tests').enabled() \
  or (get_option('tests').allowed() and not meson.is_subproject())
summary('Build tests',
  build_tests.to_string() + get_option('tests').auto().to_string(' (auto)', ''),
  section: 'Configuration',
)

msgpack_cxx_dep = dependency('msgpack-cxx',
  default_options: [
    'boost=disabled',
    'tests=disabled',
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
curl_dep = dependency('libcurl',
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
).as_system('system')
utf8cpp_dep = dependency('utf8cpp').as_system('system')

if build_server
  crow_dep = dependency('Crow').as_system('system')
  cli11_dep = dependency('cli11').as_system('system')

  if get_option('wrap_mode') != 'forcefallback' and get_option('force_fallback_for').contains('Magick++')
    magickpp_dep = dependency('Magick++', required: false)
  else
    magickpp_dep = dependency('', required: false)
  endif
  if not magickpp_dep.found()
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
    if magickpp_dep.version() == 'unknown' or magickpp_dep.version() == 'undefined' or magickpp_dep.version().version_compare('>=7.0.0')
      add_project_arguments(
        '-DWEBCFACE_MAGICK_VER7',
        language: ['c', 'cpp'],
      )
    endif
  endif
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

if cxx.get_id() == 'gcc' or cxx.get_id() == 'clang'
  warning_options = declare_dependency(compile_args: [
    '-Wno-inline',
  ])
else
  warning('Unsupported compiler id ' + cxx.get_id())
  warning_options = declare_dependency()
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
  objects: [
    webcface_encoding_lib.extract_all_objects(recursive: false),
    webcface_message_lib.extract_all_objects(recursive: false),
    webcface_client_lib.extract_all_objects(recursive: false),
  ],
  dependencies: [
    filesystem_dep,
    spdlog_dep,
    magickpp_dep,
    curl_dep,
  ],
  version: meson.project_version(),
  soversion: webcface_soversion,
)
webcface_dep = declare_dependency(
  link_with: [webcface_lib],
  include_directories: [
    webcface_config_inc,
    webcface_encoding_inc,
    webcface_client_inc,
  ],
  dependencies: [
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


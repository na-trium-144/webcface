project(
  'spdlog',
  'cpp',
  version: '1.15.3',
  license: 'MIT',
  meson_version: '>=0.56.0',
)

inc = include_directories('include')

thread_dep = dependency('threads')

spdlog_dependencies = [thread_dep]
spdlog_compile_args = []
override_cpp = meson.version().version_compare('>=1.3.0') ? 'cpp_std=gnu++17,c++17' : 'cpp_std=gnu++17'

if meson.get_compiler('cpp').get_argument_syntax() == 'msvc'
  spdlog_compile_args += '/utf-8'
endif

if meson.get_compiler('cpp').has_header_symbol(
  'format',
  '__cpp_lib_format',
  required: get_option('std_format'),
)
  spdlog_compile_args += '-DSPDLOG_USE_STD_FORMAT'
  fmt_dep = dependency(
    '',
    required: false,
  )
  override_cpp = meson.version().version_compare('>=1.3.0') ? 'cpp_std=gnu++20,c++20' : 'cpp_std=gnu++20'
else
  fmt_dep = dependency(
    'fmt',
    version: '>=11.0.0',
  )
  spdlog_dependencies += fmt_dep
  spdlog_compile_args += '-DSPDLOG_FMT_EXTERNAL'
endif

if get_option('compile_library')
  spdlog_compile_args += '-DSPDLOG_COMPILED_LIB'
  if get_option('default_library') != 'static'
    spdlog_compile_args += '-DSPDLOG_SHARED_LIB'
    spdlog_compile_args += '-Dspdlog_EXPORTS'
    if not fmt_dep.found()
      spdlog_compile_args += '-DFMT_EXPORT'
      spdlog_compile_args += '-DFMT_SHARED'
    endif
  endif
  subdir('src')

  if host_machine.system() == 'linux' and get_option('default_library') == 'static'
    # Need this to hide symbols that remains even with visibility:'inlineshidden' due to gcc bug
    spdlog_link_args = ['-Wl,--exclude-libs,' + import('fs').name(spdlog_lib.full_path())]
  else
    spdlog_link_args = []
  endif

  spdlog_dep = declare_dependency(
    include_directories: inc,
    dependencies: spdlog_dependencies,
    link_with: spdlog_lib,
    compile_args: spdlog_compile_args,
    link_args: spdlog_link_args,
  )
else
  spdlog_dep = declare_dependency(
    include_directories: inc,
    dependencies: spdlog_dependencies,
    compile_args: spdlog_compile_args,
  )
endif

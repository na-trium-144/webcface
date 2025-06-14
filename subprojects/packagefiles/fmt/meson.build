project(
  'fmt',
  'cpp',
  version: '11.2.0',
  license: 'MIT WITH fmt-exception',
  default_options: ['cpp_std=c++14'],
)

fmt_private_cpp_args = []
fmt_interface_cpp_args = []
if get_option('default_library') == 'shared'
  fmt_private_cpp_args += ['-DFMT_LIB_EXPORT']
  fmt_interface_cpp_args += ['-DFMT_SHARED']
endif
fmt_src = ['src/format.cc']

if meson.get_compiler('cpp').get_argument_syntax() == 'msvc'
  fmt_private_cpp_args += '/utf-8'
  fmt_interface_cpp_args += '/utf-8'
endif

# https://github.com/fmtlib/fmt/issues/2710
if host_machine.system() == 'cygwin'
  fmt_private_cpp_args += '-DFMT_OS=0'
else
  fmt_src += 'src/os.cc'
endif

header_only = get_option('header-only')

fmt_lib = library(
  'fmt',
  fmt_src,
  cpp_args: fmt_private_cpp_args,
  include_directories: 'include',
  build_by_default: not header_only,
  gnu_symbol_visibility: 'inlineshidden',
)

fmt_dep = declare_dependency(
  include_directories: 'include',
  compile_args: fmt_interface_cpp_args,
  link_with: fmt_lib,
)

if meson.version().version_compare('>=0.54.0')
  meson.override_dependency('fmt', fmt_dep)
endif

fmt_header_only_dep = declare_dependency(
  include_directories: 'include',
  compile_args: '-DFMT_HEADER_ONLY',
)

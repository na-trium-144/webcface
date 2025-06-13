project('crow', 'cpp')

asio_dep = dependency('asio', version: '>=1.28')

crow_dep = declare_dependency(
  include_directories: ['include'],
  dependencies: [asio_dep],
  compile_args: ['-DASIO_NO_DEPRECATED'],
)
meson.override_dependency('Crow', crow_dep)

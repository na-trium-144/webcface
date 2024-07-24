project('crow', 'cpp')

asio_dep = dependency('asio')

crow_dep = declare_dependency(
  include_directories: ['include'],
  dependencies: [asio_dep],
)
meson.override_dependency('Crow', crow_dep)

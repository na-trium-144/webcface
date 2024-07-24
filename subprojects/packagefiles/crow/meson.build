project('crow', 'cpp')

asio_dep = dependency('asio', required: false)
if not asio_dep.found()
  asio_dep = subproject('asio').get_variable('asio_dep')
endif

crow_dep = declare_dependency(
  include_directories: ['include'],
  dependencies: [asio_dep],
)

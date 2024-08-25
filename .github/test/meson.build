project('a', 'c', 'cpp',
  default_options: [
    'cpp_std=c++17',
  ],
)
webcface_dep = dependency('webcface')
executable('test-cpp', 'test.cc',
  dependencies: [webcface_dep],
)
executable('test-c', 'test.c',
  dependencies: [webcface_dep],
  link_language: 'cpp', # staticビルド時のみ必要
)

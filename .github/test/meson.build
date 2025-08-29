project('a', 'c', 'cpp',
  default_options: [
    'cpp_std=c++23,c++20,c++17',
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

cxx = meson.get_compiler('cpp')
if cxx.get_id() == 'gcc' and cxx.version().version_compare('>=11')
  webcface_miu = webcface_dep.get_variable('includedir') / 'webcface/modules/webcface.cc'
  executable('test-module-cpp', 'test_module.cc', webcface_miu,
    dependencies: [webcface_dep],
    # https://github.com/ninja-build/ninja/issues/1962
    cpp_args: ['-fmodules-ts', '-fdeps-format=p1689r5']
  )
elif cxx.get_argument_syntax() == 'msvc'
  webcface_miu = webcface_dep.get_variable('includedir') / 'webcface/modules/webcface.ixx'
  executable('test-module-cpp', 'test_module.cc', webcface_miu,
    dependencies: [webcface_dep],
  )
endif

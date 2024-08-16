webcface_launcher_src = [
  'src/launcher.cc',
  'src/command.cc',
]
webcface_launcher_inc = include_directories('include')
webcface_launcher_lib = static_library('webcface-launcher',
  sources: webcface_launcher_src,
  include_directories: [
    webcface_launcher_inc,
  ],
  dependencies: [
    webcface_config_dep,
    warning_options,
    # webcface_dep,
    webcface_encoding_dep,
    webcface_message_dep,
    webcface_client_dep,
    filesystem_dep,
    spdlog_dep,
    tomlplusplus_dep,
    tiny_process_library_dep,
  ],
  cpp_args: ['-DWEBCFACE_BUILDING'],
)
webcface_launcher_dep = declare_dependency(
  include_directories: [
    webcface_launcher_inc,
  ],
  dependencies: [
    # webcface_dep,
    webcface_config_dep,
    webcface_encoding_dep,
    webcface_client_dep,
    filesystem_dep,
    spdlog_dep,
  ],
)

webcface_server_store_src = [
  'src/member_data.cc',
  'src/image_conversion.cc',
  'src/store.cc',
  'src/websock.cc',
  'src/dir.cc',
  'src/ip.cc',
  'src/server_ws.cc',
]
webcface_server_store_inc = include_directories('include')
if webcface_system_win32socket
  win32socket_deps = [
    cxx.find_library('iphlpapi'),
    cxx.find_library('ws2_32'),
    cxx.find_library('wsock32'), # asioの依存ライブラリなのでasioに追加されているべきでは?
  ]
else
  win32socket_deps = []
endif
webcface_server_store_lib = static_library('webcface-server-store',
  sources: webcface_server_store_src,
  include_directories: [
    webcface_server_store_inc,
  ],
  dependencies: [
    webcface_config_dep,
    warning_options,
    # webcface_dep,
    webcface_encoding_dep,
    webcface_message_dep,
    webcface_client_dep,
    webcface_launcher_dep,
    filesystem_dep,
    spdlog_dep,
    magickpp_dep,
    crow_dep,
    win32socket_deps,
  ],
  cpp_args: ['-DWEBCFACE_BUILDING'],
)
webcface_server_store_dep = declare_dependency(
  include_directories: [
    webcface_server_store_inc,
  ],
  dependencies: [
    # webcface_dep,
    webcface_config_dep,
    webcface_encoding_dep,
    webcface_launcher_dep,
    filesystem_dep,
    spdlog_dep,
    magickpp_dep,
    win32socket_deps,
  ],
)

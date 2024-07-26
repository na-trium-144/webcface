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
  ip = cxx.find_library('iphlpapi')
else
  ip = declare_dependency()
endif
webcface_server_store_lib = static_library('webcface-server-store',
  sources: webcface_server_store_src,
  include_directories: [
    webcface_config_inc,
    webcface_server_store_inc,
  ],
  dependencies: [
    webcface_dep,
    filesystem_dep,
    webcface_message_dep,
    spdlog_dep,
    magickpp_dep,
    crow_dep,
    ip,
  ],
)
webcface_server_store_dep = declare_dependency(
  include_directories: [
    webcface_server_store_inc,
  ],
  link_with: [
    webcface_server_store_lib,
  ],
  dependencies: [
    webcface_dep,
    spdlog_dep,
  ],
)

executable('webcface-server',
  sources: [
    'server_main.cc',
  ],
  dependencies: [
    warning_options,
    # webcface_dep,
    webcface_server_store_dep,
    cli11_dep,
  ],
  cpp_args: ['-DWEBCFACE_BUILDING'],
  link_with: [
    webcface_encoding_lib,
    webcface_message_lib,
    webcface_client_lib,
    webcface_server_store_lib,
  ],
  install: true,
)

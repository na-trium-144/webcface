sources = [
  'server_main.cc',
]
if webcface_system_version_rc
  sources += import('windows').compile_resources(
    configure_file(
      input: version_rc_in,
      output: 'server_version.rc',
      configuration: configuration_data({
        'company_name': webcface_contact,
        'file_description': 'WebCFace Server',
        'filetype_app': 1,
      }),
    ),
    include_directories: [webcface_config_inc],
  )
endif
executable('webcface-server',
  sources: sources,
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

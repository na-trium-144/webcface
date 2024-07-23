webcface_message_src = [
  'src/message.cc',
]
webcface_message_inc = include_directories('include')
webcface_message_lib = static_library('webcface-message',
  sources: webcface_message_src,
  include_directories: [
    webcface_config_inc,
    webcface_encoding_inc,
    webcface_message_inc,
  ],
  build_by_default: false,
  cpp_args: ['-DWEBCFACE_BUILDING'],
)

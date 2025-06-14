# 1ソースファイルのサイズが大きすぎるとmingwで大量のundefined referenceが起きる。
# エラーが起きたら分割することにしたが、解決するリンクオプションとかあるのかな?
# 分割したらmsvcで/bigobjフラグが要らなくなった。関係ありそう
webcface_client_src = [
  'src/client_threading.cc',
  'src/client_msg_sync.cc',
  'src/client_msg_recv.cc',
  'src/client_ws.cc',
  'src/data_store1.cc',
  'src/data_store2.cc',
  'src/field.cc',
  'src/func_info.cc',
  'src/func_result.cc',
  'src/func.cc',
  'src/func_listener.cc',
  'src/log.cc',
  'src/member.cc',
  'src/text.cc',
  'src/robot_link.cc',
  'src/robot_model.cc',
  'src/canvas3d.cc',
  'src/canvas2d.cc',
  'src/value.cc',
  'src/view.cc',
  'src/plot.cc',
  'src/image.cc',
  'src/image_frame.cc',
  'src/version.cc',
  'src/component_canvas3d.cc',
  'src/component_canvas2d.cc',
  'src/component_view.cc',
  'src/transform.cc',
  'src/c_wcf/client.cc',
  'src/c_wcf/member.cc',
  'src/c_wcf/value.cc',
  'src/c_wcf/text.cc',
  'src/c_wcf/func.cc',
  'src/c_wcf/view.cc',
]
webcface_client_inc = include_directories('include')
webcface_client_lib = static_library('webcface-client',
  sources: webcface_client_src,
  include_directories: [
    webcface_client_inc,
  ],
  dependencies: [
    webcface_config_dep,
    wabi_options_dep,
    warning_options_dep,
    filesystem_dep,
    webcface_common_dep,
    spdlog_dep,
    curl_dep,
    m_dep,
  ],
  build_by_default: false,
  cpp_args: ['-DWEBCFACE_BUILDING'],
  gnu_symbol_visibility: 'inlineshidden',
)
webcface_client_dep = declare_dependency(
  include_directories: [
    webcface_client_inc,
  ],
  dependencies: [
    filesystem_dep,
    webcface_common_dep,
  ],
)

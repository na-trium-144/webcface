example_src = [
  {'src': 'value.cc', 'libvips': false, 'cv': false},
  {'src': 'func.cc', 'libvips': false, 'cv': false},
  {'src': 'log.cc', 'libvips': false, 'cv': false},
  {'src': 'view.cc', 'libvips': false, 'cv': false},
  {'src': 'canvas2d.cc', 'libvips': false, 'cv': false},
  {'src': 'canvas3d.cc', 'libvips': false, 'cv': false},
  {'src': 'recv.cc', 'libvips': false, 'cv': false},
  {'src': 'c_example.c', 'libvips': false, 'cv': false},
  {'src': 'benchmark.cc', 'libvips': false, 'cv': false},
  {'src': 'image-send.cc', 'libvips': true, 'cv': false},
  {'src': 'image-recv.cc', 'libvips': true, 'cv': false},
  {'src': 'cv-send.cc', 'libvips': false, 'cv': true},
  {'src': 'cv-recv.cc', 'libvips': false, 'cv': true},
  {'src': 'tutorial-vis.cc', 'libvips': false, 'cv': false},
  {'src': 'tutorial-send.cc', 'libvips': false, 'cv': false},
  {'src': 'tutorial-recv.cc', 'libvips': false, 'cv': false},
  {'src': 'joystick-recv.cc', 'libvips': false, 'cv': false},
]

foreach e: example_src
  target = fs.replace_suffix(e['src'], '')
  sources = [e['src']]
  dependencies = [
    warning_options_dep,
    webcface_dep,
    fmt_dep,
  ]
  if e['libvips']
    dependencies += libvips_cpp_dep
  endif
  if e['cv']
    dependencies += [
      opencv_core_dep,
      opencv_imgproc_dep,
      opencv_highgui_dep,
    ]
  endif
  if webcface_system_version_rc
    sources += import('windows').compile_resources(
      configure_file(
        input: version_rc_in,
        output: target + '_version.rc',
        configuration: configuration_data({
          'company_name': webcface_contact,
          'file_description': 'WebCFace Example ' + target,
          'filetype_app': 1,
        }),
      ),
      include_directories: [webcface_config_inc],
    )
  endif
  executable('webcface-example-' + target,
    sources: sources,
    dependencies: dependencies,
    install: true,
    install_rpath: relative_rpath,
  )
endforeach

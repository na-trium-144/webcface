example_src = [
  {'src': 'value.cc', 'magick': false, 'cv': false},
  {'src': 'func.cc', 'magick': false, 'cv': false},
  {'src': 'log.cc', 'magick': false, 'cv': false},
  {'src': 'view.cc', 'magick': false, 'cv': false},
  {'src': 'canvas2d.cc', 'magick': false, 'cv': false},
  {'src': 'canvas3d.cc', 'magick': false, 'cv': false},
  {'src': 'recv.cc', 'magick': false, 'cv': false},
  {'src': 'c_example.c', 'magick': false, 'cv': false},
  {'src': 'benchmark.cc', 'magick': false, 'cv': false},
  {'src': 'image-send.cc', 'magick': true, 'cv': false},
  {'src': 'image-recv.cc', 'magick': true, 'cv': false},
  {'src': 'cv-send.cc', 'magick': false, 'cv': true},
  {'src': 'cv-recv.cc', 'magick': false, 'cv': true},
  {'src': 'tutorial-vis.cc', 'magick': false, 'cv': false},
]

foreach e: example_src
  target = fs.replace_suffix(e['src'], '')
  sources = [e['src']]
  dependencies = [
    warning_options,
    webcface_dep,
  ]
  if e['magick']
    dependencies += magickpp_dep
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

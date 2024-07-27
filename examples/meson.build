example_src = [
  'value.cc',
  'func.cc',
  'log.cc',
  'view.cc',
  'canvas2d.cc',
  'canvas3d.cc',
  'recv.cc',
  'c_example.c',
  'benchmark.cc',
]
example_magick_src = [
  'image-send.cc',
  'image-recv.cc',
]
example_cv_src = [
  'cv-send.cc',
  'cv-recv.cc',
]

foreach src: example_src
  executable('webcface-example-' + fs.replace_suffix(src, ''),
    sources: src,
    dependencies: [
      webcface_dep,
    ]
  )
endforeach
if build_server
  foreach src: example_magick_src
    executable('webcface-example-' + fs.replace_suffix(src, ''),
      sources: src,
      dependencies: [
        webcface_dep,
        magickpp_dep,
      ]
    )
  endforeach
endif
if opencv_dep.found()
  foreach src: example_cv_src
    executable('webcface-example-' + fs.replace_suffix(src, ''),
      sources: src,
      dependencies: [
        webcface_dep,
        opencv_dep,
      ]
    )
  endforeach
endif

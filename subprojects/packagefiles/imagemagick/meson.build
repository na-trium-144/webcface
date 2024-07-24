project('imagemagick', 'c', 'cpp',
  meson_version : '>=0.63.0',
)

magickpp_dependencies = []

libjpeg_dep = dependency('libjpeg', required: false)
if not libjpeg_dep.found()
  libjpeg_proj = subproject('libjpeg-turbo')
  libjpeg_dep = libjpeg_proj.get_variable('jpeg_dep')
  magickpp_dependencies += libjpeg_proj.get_variable('jpeg')
endif
zlib_dep = dependency('zlib', required: false)
if not zlib_dep.found()
  zlib_proj = subproject('zlib')
  zlib_dep = zlib_proj.get_variable('zlib_dep')
  magickpp_dependencies += zlib_proj.get_variable('zlib')
endif
libpng_dep = dependency('libpng', required: false)
if not libpng_dep.found()
  libpng_proj = subproject('libpng')
  libpng_dep = get_variable('libpng_dep')
  magickpp_dependencies += libpng_proj.get_variable('libpng')
endif
libwebp_dep = dependency('libwebp', required: false)
if not libwebp_dep.found()
  libwebp_proj = subproject('libwebp')
  libwebp_dep = libwebp_proj.get_variable('webp_dep')
  magickpp_dependencies += libwebp_proj.get_variable('webp_lib')
endif

mod = import('unstable-external_project')

p = mod.add_project('configure',
  configure_options: [
    '--prefix=@PREFIX@',
    '--disable-shared',
    '--enable-static',
    '--disable-openmp', # <- for multi threading
    '--without-utilities',
    '--disable-hdri',
    '--with-quantum-depth=8',
    '--without-modules',
    '--without-perl',
    '--without-bzlib',
    '--without-djvu',
    '--without-dps',
    '--without-fontconfig',
    '--without-freetype',
    '--without-gvc',
    '--without-heic',
    '--without-jbig',
    '--without-jxl',
    '--without-dmr',
    '--without-lqr',
    '--without-lcms',
    '--without-lzma',
    '--without-openexr',
    '--without-openjp2',
    '--without-pango',
    '--without-raqm',
    '--without-raw',
    '--without-tiff',
    '--without-wmf',
    '--without-xml',
    '--without-zlib',
    '--without-zstd',
    '--without-x',
    '--without-zip',
  ],
  env: [
    'ac_cv_lib_jpeg_jpeg_read_header=yes', # 無理やりチェック通ったことにする
  ],
  depends: magickpp_dependencies,
)

magickpp_dep = declare_dependency(
  dependencies: [
    p.dependency('Magick++'),
    libwebp_dep,
    libpng_dep,
    zlib_dep,
    libjpeg_dep,
  ],
)

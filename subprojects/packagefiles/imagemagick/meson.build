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
if libwebp_dep.found()
  libwebpmux_dep = dependency('libwebpmux', required: false)
  libwebpdemux_dep = dependency('libwebpdemux', required: false)
else
  libwebp_proj = subproject('libwebp')
  libwebp_dep = libwebp_proj.get_variable('webp_dep')
  magickpp_dependencies += libwebp_proj.get_variable('webp_lib')
  libwebpmux_dep = libwebp_proj.get_variable('webpmux_dep')
  magickpp_dependencies += libwebp_proj.get_variable('webpmux_lib')
  libwebpdemux_dep = libwebp_proj.get_variable('webpdemux_dep')
  magickpp_dependencies += libwebp_proj.get_variable('webpdemux_lib')
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
  verbose: true,
)

pkg_config = find_program('pkg-config')
magickcore_cflags = run_command(
  pkg_config, '--cflags-only-other', 'MagickCore',
  env: {
    'PKG_CONFIG_PATH': meson.current_build_dir() / 'build/MagickCore',
  },
  check: true,
)
magick_inc_subdir = 'ImageMagick-7'
magickcore_dep = declare_dependency(
  compile_args: magickcore_cflags.stdout().split(),
  dependencies: [
    p.dependency('MagickCore-7.Q8',
      subdir: magick_inc_subdir,
    ),
    libwebp_dep,
    libwebpmux_dep,
    libwebpdemux_dep,
    libpng_dep,
    zlib_dep,
    libjpeg_dep,
  ],
)
# pkg-config MagickWand はdependencyが見つからないエラーになる
# MagickWandとMagick++に追加のcflagsはなさそうなのでok
magickwand_dep = declare_dependency(
  dependencies: [
    p.dependency('MagickWand-7.Q8',
      subdir: magick_inc_subdir,
    ),
    magickcore_dep,
  ],
)
magickpp_dep = declare_dependency(
  dependencies: [
    p.dependency('Magick++-7.Q8',
      subdir: magick_inc_subdir,
    ),
    magickwand_dep,
  ],
)
meson.override_dependency('MagickCore', magickcore_dep)
meson.override_dependency('MagickWand', magickwand_dep)
meson.override_dependency('Magick++', magickpp_dep)

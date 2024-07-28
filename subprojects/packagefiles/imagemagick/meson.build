project('imagemagick', 'c', 'cpp',
  meson_version : '>=0.63.0',
)

pkg_config = find_program('pkg-config')
fs = import('fs')
cxx = meson.get_compiler('cpp')
sed = find_program('sed')
cp = find_program('cp')
mv = find_program('mv')

# Make.comというファイルがあるせいでmakeコマンドを誤認識する
if fs.exists('Make.com')
  run_command(mv, 'Make.com', 'Make.com_', check: true)
endif

# configureを別名にし、複数のビルドで干渉しないようにする
new_configure_name = 'configure_' + fs.name(meson.global_build_root())

# configureファイルを毎回新しく生成してもそのままにしても、
# 結局configureを実行するたびにすべて再ビルドすることになる

message('copying configure to ' + new_configure_name)
run_command(cp, 'configure', new_configure_name, check: true)

if build_machine.system() == 'windows'
  # mingwでは$0がwindowsパスの場合なぜかうまく動作しないので、configureを書き換える
  cygpath = find_program('cygpath', required: false)
  if cygpath.found()
    run_command(sed, '-i',
      '-e', 's/\$0/$self/g',
      '-e', '1a self=$(cygpath -u "$0")',
      '-e', '/^cat >\$CONFIG_STATUS.*/a self=$(cygpath -u "$0")',
      new_configure_name,
      check: true,
    )
  endif
  run_command(sed, '-i',
    # winpath.sh: permission denied になることがある
    '-e', 's/\$WinPathScript/sh $WinPathScript/g',
    new_configure_name,
    check: true,
  )
endif

magickpp_dependencies = []
magickpp_config_vars = []

libjpeg_dep = dependency('libjpeg', required: false)
if not libjpeg_dep.found() or libjpeg_dep.type_name() == 'internal'
  libjpeg_proj = subproject('libjpeg-turbo')
  libjpeg_dep = libjpeg_proj.get_variable('jpeg_dep')
  magickpp_dependencies += libjpeg_proj.get_variable('jpeg')
  # 無理やりチェック通ったことにする
  magickpp_config_vars += 'ac_cv_lib_jpeg_jpeg_read_header=yes'
endif

zlib_dep = dependency('zlib', required: false)
if not zlib_dep.found() or zlib_dep.type_name() == 'internal'
  zlib_proj = subproject('zlib')
  zlib_dep = zlib_proj.get_variable('zlib_dep')
  magickpp_dependencies += zlib_proj.get_variable('zlib')
endif

libpng_dep = dependency('libpng', required: false)
if not libpng_dep.found() or libpng_dep.type_name() == 'internal'
  libpng_proj = subproject('libpng')
  libpng_dep = get_variable('libpng_dep')
  magickpp_dependencies += libpng_proj.get_variable('libpng')
  # libpngのwrapがpc.generate()してくれずmagickppがpngを見つけることができないので、でっちあげをする
  libpng_pc_cdata = configuration_data()
  libpng_pc_cdata.merge_from(libpng_proj.get_variable('cdata'))
  libpng_pc_cdata.set('builddir', meson.current_build_dir() / '../libpng-' + libpng_dep.version())
  libpng_pc_cdata.set('srcdir', meson.global_source_root() / 'subprojects' / 'libpng-' + libpng_dep.version())
  configure_file(
    input: 'libpng-uninstalled.pc.in',
    output: 'libpng-uninstalled.pc',
    configuration: libpng_pc_cdata,
    install: false,
  )
  run_command(cp,
    meson.current_build_dir() / 'libpng-uninstalled.pc',
    meson.global_build_root() / 'meson-uninstalled',
    check: true,
  )
endif

libwebp_dep = dependency('libwebp', required: false)
if libwebp_dep.found() and libwebp_dep.type_name() != 'internal'
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

p = mod.add_project(new_configure_name,
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
  env: magickpp_config_vars,
  depends: magickpp_dependencies,
  verbose: true,
)

if host_machine.system() == 'windows'
  urlmon = cxx.find_library('urlmon')
else
  urlmon = declare_dependency()
endif
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
    urlmon,
    libwebp_dep,
    libwebpmux_dep,
    libwebpdemux_dep,
    libpng_dep,
    zlib_dep,
    libjpeg_dep,
    dependency('threads'),
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

if not fs.exists('Make.com')
  # もとに戻しておかないとエラー なんで?
  run_command(cp, 'Make.com_', 'Make.com', check: true)
endif

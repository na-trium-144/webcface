project('cpp-filesystem', 'cpp')

# libc++fsやlibc++experimentalが必要になる環境でのビルドはテストしていない

# meson側で実装してくれたらいいなあ https://github.com/mesonbuild/meson/issues/14249

cxx = meson.get_compiler('cpp')
cpp_filesystem_dep = disabler()
experimental = false

filesystem_check_src = '''
#if defined(CXX17_FILESYSTEM)
#include <filesystem>
namespace std_fs = std::filesystem;
#else
#include <experimental/filesystem>
namespace std_fs = std::experimental::filesystem;
#endif
int main(void) {
    return std_fs::exists(std_fs::path("/")), 0;
}
'''

foreach fs_include : ['C++17', 'experimental']
  experimental = fs_include != 'C++17'
  if not meson.can_run_host_binaries()
    fs_compiles = cxx.compiles(filesystem_check_src,
      args: fs_include == 'C++17' ? ['-DCXX17_FILESYSTEM'] : [],
      name: fs_include + ' filesystem',
    )
    if fs_compiles
      cpp_filesystem_dep = declare_dependency()
      summary('filesystem', fs_include)
      summary('additional libs', 'skipped due to cross compiling')
      break
    endif
  else
    fs_ret = cxx.run(filesystem_check_src,
      args: fs_include == 'C++17' ? ['-DCXX17_FILESYSTEM'] : [],
      name: fs_include + ' filesystem',
    )
    if fs_ret.compiled() and fs_ret.returncode() == 0
      cpp_filesystem_dep = declare_dependency()
      summary('filesystem', fs_include)
      summary('additional libs', false, bool_yn: true)
      break
    endif
    foreach libfs_name : ['stdc++fs', 'c++fs', 'c++experimental']
      libfs = cxx.find_library(libfs_name, required: false)
      if libfs.found()
        libfs_ret = cxx.run(filesystem_check_src,
          args: fs_include == 'C++17' ? ['-DCXX17_FILESYSTEM'] : [],
          name: fs_include + ' filesystem with ' + libfs_name,
          dependencies: [libfs],
        )
        if libfs_ret.compiled() and libfs_ret.returncode() == 0
          cpp_filesystem_dep = libfs
          summary('filesystem', fs_include)
          summary('additional libs', libfs_name)
          break
        endif
      endif
    endforeach
  endif
endforeach

if is_disabler(cpp_filesystem_dep)
  error('Neither std::filesystem or std::experimental::filesystem available')
endif

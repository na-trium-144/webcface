include(CMakePackageConfigHelpers)

set(CMAKE_SIZEOF_VOID_P ${sizeof_void_p})
write_basic_package_version_file(
    ${build_dir}/webcface-config-version.cmake
    VERSION ${version}
    COMPATIBILITY SameMajorVersion
)

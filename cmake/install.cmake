# https://qiita.com/modapone/items/8f97425b6167cffc815c

include(CMakePackageConfigHelpers)

install(TARGETS ${PROJECT_NAME}
    EXPORT ${PROJECT_NAME}-targets
    # 静的ライブラリのインストール先
    ARCHIVE DESTINATION lib
    # 共有ライブラリのインストール先
    LIBRARY DESTINATION lib)

# ++
    # インストールするconfigの指定、前項で生成したもの
install(EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    # 名前空間の指定
    NAMESPACE ${PROJECT_NAME}::
    # インストール先
    DESTINATION lib/cmake/${PROJECT_NAME})
# # ++
    # コピー元、末尾の`/`は要注意
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/inc/
    # コピー先
    DESTINATION include)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/frontend/out/
    DESTINATION share/${PROJECT_NAME}/frontend/out)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/generator/
    DESTINATION share/${PROJECT_NAME}/generator)

# ++
set(INSTALL_DATA_DIR ${CMAKE_INSTALL_DATAROOTDIR}/${PROJECT_NAME})
message("${INSTALL_DATA_DIR}")
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
  PATH_VARS INSTALL_DATA_DIR
)

# Package Version Fileの生成
write_basic_package_version_file(
    # 生成するファイルの名前
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    # 互換性の判定方法の指定
    COMPATIBILITY SameMajorVersion)
    # バージョンの指定、project(...)で指定しているため省略可能
#   VERSION 0.1)

# Package Version Fileのインストール
    # コピー元
install(FILES 
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/generate.cmake
    # コピー先
    DESTINATION lib/cmake/${PROJECT_NAME})


# pywebcface
if(DEFINED WEBCFACE_PYTHON_INSTALL)
    if(${WEBCFACE_PYTHON_INSTALL})
        install(TARGETS pywebcface
            LIBRARY DESTINATION python/webcface)
        configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/version.py.in
            ${CMAKE_CURRENT_SOURCE_DIR}/python/webcface/version.py
        )
    endif()
endif()

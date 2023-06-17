file(GLOB_RECURSE FRONTEND_SOURCES
    ${CMAKE_CURRENT_LIST_DIR}/../frontend/components/*
    ${CMAKE_CURRENT_LIST_DIR}/../frontend/lib/*
    ${CMAKE_CURRENT_LIST_DIR}/../frontend/pages/*
    ${CMAKE_CURRENT_LIST_DIR}/../frontend/public/*
    ${CMAKE_CURRENT_LIST_DIR}/../frontend/styles/*
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/frontend_install
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../frontend
  COMMAND npm install
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/frontend_install
  DEPENDS ${CMAKE_CURRENT_LIST_DIR}/../frontend/package.json
  COMMENT "npm install"
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/frontend_build
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../frontend
  COMMAND npm run build
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/frontend_build
  DEPENDS ${FRONTEND_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/frontend_install
  COMMENT "npm run build"
)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/frontend_export
  WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../frontend
  COMMAND npm run export
  COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/frontend_export
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/frontend_build
  COMMENT "npm run export"
)
add_custom_target(frontend DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/frontend_export)
if(${WEBCFACE_BUILD_FRONT})
    add_dependencies(${PROJECT_NAME} frontend)
else()
message("フロントエンドは自動ビルドされません")
message("make frontend / ninja frontend でビルドできます")
endif()
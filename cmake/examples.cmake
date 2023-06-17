find_package(OpenCV)
if(NOT OpenCV_FOUND)
  message("OpenCVが見つからなかったので、ウェブカメラの機能はDisableされます")
endif()

file(GLOB EXAMPLE_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/examples/*.cpp)
foreach(EXAMPLE_FILE IN LISTS EXAMPLE_SOURCE)
  get_filename_component(EXAMPLE_TARGET ${EXAMPLE_FILE} NAME_WE)
  # message("new example : ${EXAMPLE_TARGET}")
  add_executable(${EXAMPLE_TARGET} ${EXAMPLE_FILE})
  if(OpenCV_FOUND)
    target_include_directories(${EXAMPLE_TARGET} PRIVATE ${OpenCV_INCLUDE_DIRS})
    target_link_libraries(${EXAMPLE_TARGET} PRIVATE ${OpenCV_LIBS})
    target_compile_definitions(${EXAMPLE_TARGET} PRIVATE WEBCFACE_OPENCV_FOUND)
  endif()
  target_link_libraries(${EXAMPLE_TARGET} PRIVATE webcface)
endforeach()

add_executable(example_linking
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/linking/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/examples/linking/func.cpp
)
target_link_libraries(example_linking PRIVATE webcface)


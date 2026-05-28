cmake_minimum_required(VERSION 3.10)
project(lightstereo_demo)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RKNN_LIB_DIR ${CMAKE_SOURCE_DIR}/third_party/rknn/lib/aarch64)
set(OPENCV_DIR ${CMAKE_SOURCE_DIR}/third_party/opencv/opencv-linux-aarch64)

find_package(OpenCV REQUIRED PATHS ${OPENCV_DIR} NO_DEFAULT_PATH)

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/third_party/rknn/include
  ${OpenCV_INCLUDE_DIRS}
)

link_directories(${RKNN_LIB_DIR})

add_library(lightstereo_core STATIC
  src/lightstereo_infer.cpp
  src/postprocess.cpp
  src/rknn_utils.cpp
  src/image_utils.cpp
)

target_link_libraries(lightstereo_core
  rknnrt
  ${OpenCV_LIBS}
)

add_executable(lightstereo_demo app/main.cpp)
target_link_libraries(lightstereo_demo lightstereo_core)

install(TARGETS lightstereo_demo DESTINATION ${CMAKE_SOURCE_DIR}/install/lightstereo_rk3588)
install(DIRECTORY models/ DESTINATION ${CMAKE_SOURCE_DIR}/install/lightstereo_rk3588/models)
install(DIRECTORY samples/ DESTINATION ${CMAKE_SOURCE_DIR}/install/lightstereo_rk3588/samples)
install(DIRECTORY ${RKNN_LIB_DIR}/ DESTINATION ${CMAKE_SOURCE_DIR}/install/lightstereo_rk3588/lib FILES_MATCHING PATTERN "*.so")

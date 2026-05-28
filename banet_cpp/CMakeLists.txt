cmake_minimum_required(VERSION 3.10)
project(banet2d_rk3588 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

option(BANET2D_VENDOR_DEPS "Use dependencies vendored under third_party" ON)
set(RKNPU2_ROOT "" CACHE PATH "RKNPU2 root for external dependency mode")

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

if(BANET2D_VENDOR_DEPS)
  set(RKNN_INCLUDE_DIR ${PROJECT_ROOT}/third_party/rknn/include)
  set(RKNN_RT_LIB ${PROJECT_ROOT}/third_party/rknn/lib/aarch64/librknnrt.so)
  set(OpenCV_DIR ${PROJECT_ROOT}/third_party/opencv/opencv-linux-aarch64/share/OpenCV)
else()
  if(NOT RKNPU2_ROOT)
    message(FATAL_ERROR "Set -DRKNPU2_ROOT=/path/to/rknpu2 when BANET2D_VENDOR_DEPS=OFF")
  endif()
  set(RKNN_INCLUDE_DIR ${RKNPU2_ROOT}/runtime/RK3588/Linux/librknn_api/include)
  set(RKNN_RT_LIB ${RKNPU2_ROOT}/runtime/RK3588/Linux/librknn_api/aarch64/librknnrt.so)
  set(OpenCV_DIR ${RKNPU2_ROOT}/examples/3rdparty/opencv/opencv-linux-aarch64/share/OpenCV)
endif()

if(NOT EXISTS ${RKNN_INCLUDE_DIR}/rknn_api.h)
  message(FATAL_ERROR "rknn_api.h not found: ${RKNN_INCLUDE_DIR}/rknn_api.h")
endif()
if(NOT EXISTS ${RKNN_RT_LIB})
  message(FATAL_ERROR "librknnrt.so not found: ${RKNN_RT_LIB}")
endif()

find_package(OpenCV REQUIRED COMPONENTS core imgcodecs imgproc)

add_library(banet2d_core
  src/banet2d_infer.cpp
  src/image_utils.cpp
  src/postprocess.cpp
  src/rknn_utils.cpp
)

target_include_directories(banet2d_core
  PUBLIC
    ${PROJECT_ROOT}/include
    ${RKNN_INCLUDE_DIR}
)

target_link_libraries(banet2d_core
  PUBLIC
    ${OpenCV_LIBS}
  PRIVATE
    ${RKNN_RT_LIB}
)

add_executable(banet2d_demo app/main.cpp)
target_link_libraries(banet2d_demo PRIVATE banet2d_core ${RKNN_RT_LIB})

install(TARGETS banet2d_demo RUNTIME DESTINATION bin)
install(FILES ${RKNN_RT_LIB} DESTINATION lib)

if(BANET2D_VENDOR_DEPS)
  install(DIRECTORY ${PROJECT_ROOT}/third_party/opencv/opencv-linux-aarch64/lib/
          DESTINATION lib
          FILES_MATCHING
          PATTERN "*.so*"
          PATTERN "*.a")
endif()

install(DIRECTORY ${PROJECT_ROOT}/models/ DESTINATION models FILES_MATCHING PATTERN "*.rknn")
install(DIRECTORY ${PROJECT_ROOT}/samples/ DESTINATION samples FILES_MATCHING PATTERN "*.png" PATTERN "*.jpg" PATTERN "*.jpeg")
install(PROGRAMS
  ${PROJECT_ROOT}/scripts/run_demo.sh
  ${PROJECT_ROOT}/scripts/check_runtime.sh
  DESTINATION scripts
)
install(FILES ${PROJECT_ROOT}/README.md DESTINATION .)
install(DIRECTORY DESTINATION output)

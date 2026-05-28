#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
RKNPU2_ROOT="${1:-/data2/shendu/code/ruoyu/rknpu2}"
APP_ROOT="${APP_ROOT:-/data2/shendu/app/banet2d}"

RKNN_API_DIR="$RKNPU2_ROOT/runtime/RK3588/Linux/librknn_api"
OPENCV_DIR="$RKNPU2_ROOT/examples/3rdparty/opencv/opencv-linux-aarch64"
MODEL_SRC="$APP_ROOT/checkpoint_epoch_195_int8.rknn"
LEFT_SRC="$APP_ROOT/calib/left/0000_left.png"
RIGHT_SRC="$APP_ROOT/calib/right/0000_right.png"

required=(
  "$RKNN_API_DIR/include/rknn_api.h"
  "$RKNN_API_DIR/aarch64/librknnrt.so"
  "$OPENCV_DIR/share/OpenCV/OpenCVConfig.cmake"
  "$MODEL_SRC"
  "$LEFT_SRC"
  "$RIGHT_SRC"
)

for path in "${required[@]}"; do
  if [[ ! -e "$path" ]]; then
    echo "missing required file: $path" >&2
    exit 1
  fi
done

mkdir -p \
  "$ROOT_DIR/third_party/rknn/include" \
  "$ROOT_DIR/third_party/rknn/lib/aarch64" \
  "$ROOT_DIR/third_party/opencv" \
  "$ROOT_DIR/models" \
  "$ROOT_DIR/samples"

cp -a "$RKNN_API_DIR/include/rknn_api.h" "$ROOT_DIR/third_party/rknn/include/"
cp -a "$RKNN_API_DIR/aarch64/librknnrt.so" "$ROOT_DIR/third_party/rknn/lib/aarch64/"
rm -rf "$ROOT_DIR/third_party/opencv/opencv-linux-aarch64"
cp -a "$OPENCV_DIR" "$ROOT_DIR/third_party/opencv/"
cp -a "$MODEL_SRC" "$ROOT_DIR/models/checkpoint_epoch_195_int8.rknn"
cp -a "$LEFT_SRC" "$ROOT_DIR/samples/left.png"
cp -a "$RIGHT_SRC" "$ROOT_DIR/samples/right.png"

echo "prepared vendored dependencies under: $ROOT_DIR"

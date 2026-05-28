#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

ok=1

if [[ "$(uname -m)" != "aarch64" ]]; then
  echo "warning: current arch is $(uname -m), expected aarch64 for RK3588 runtime"
fi

if ! compgen -G "/dev/rknpu*" >/dev/null; then
  echo "missing /dev/rknpu* device"
  ok=0
fi

required=(
  "bin/banet2d_demo"
  "models/checkpoint_epoch_195_int8.rknn"
  "samples/left.png"
  "samples/right.png"
  "lib/librknnrt.so"
)

for path in "${required[@]}"; do
  if [[ ! -e "$path" ]]; then
    echo "missing: $path"
    ok=0
  fi
done

if ! compgen -G "lib/libopencv_*.so*" >/dev/null && ! compgen -G "lib/libopencv_*.a" >/dev/null; then
  echo "missing OpenCV runtime/static libraries: lib/libopencv_*.so* or lib/libopencv_*.a"
  ok=0
fi

if [[ -x bin/banet2d_demo ]]; then
  export LD_LIBRARY_PATH="$ROOT_DIR/lib:${LD_LIBRARY_PATH:-}"
  echo "ldd bin/banet2d_demo:"
  ldd ./bin/banet2d_demo || ok=0
  if ldd ./bin/banet2d_demo 2>/dev/null | grep -q "not found"; then
    echo "missing shared libraries reported by ldd"
    ok=0
  fi
fi

if [[ "$ok" -ne 1 ]]; then
  exit 1
fi

echo "runtime check passed"

#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBANET2D_VENDOR_DEPS=ON
cmake --build build -j"$(nproc)"
cmake --install build --prefix install/banet2d_rk3588

echo "installed to: $ROOT_DIR/install/banet2d_rk3588"

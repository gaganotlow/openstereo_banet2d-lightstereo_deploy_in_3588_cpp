#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

export LD_LIBRARY_PATH="$ROOT_DIR/lib:${LD_LIBRARY_PATH:-}"
mkdir -p output

./bin/banet2d_demo \
  --model ./models/checkpoint_epoch_195_int8.rknn \
  --left ./samples/left.png \
  --right ./samples/right.png \
  --output ./output/demo \
  --warmup 1 \
  --repeat 1

#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
INSTALL_DIR="$ROOT_DIR/install/banet2d_rk3588"
PACKAGE_DIR="$ROOT_DIR/package"
PACKAGE_PATH="$PACKAGE_DIR/banet2d_rk3588_deploy.tar.gz"

if [[ ! -d "$INSTALL_DIR" ]]; then
  echo "missing install tree: $INSTALL_DIR" >&2
  echo "run ./scripts/build_on_board.sh first" >&2
  exit 1
fi

mkdir -p "$PACKAGE_DIR"
tar -C "$ROOT_DIR/install" -czf "$PACKAGE_PATH" banet2d_rk3588

echo "created: $PACKAGE_PATH"

#!/usr/bin/env bash
set -e

BUILD_DIR="$(dirname "$0")/build"

cmake --build "$BUILD_DIR"
ctest --test-dir "$BUILD_DIR" --output-on-failure "$@"

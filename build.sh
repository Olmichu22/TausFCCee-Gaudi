#!/bin/bash
# Configure (if needed), build and install TauTool and k4TauAnalysis, then run the tests.
set -e
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# Load key4hep only if it is not already set up in this shell
if [ -z "${KEY4HEP_STACK:-}" ]; then
  source "$ROOT_DIR/setupKey4Hep.sh"
fi
# TauTool
cd "$ROOT_DIR/TauTool"
[ -f build/CMakeCache.txt ] || cmake -S . -B build -G Ninja -DCMAKE_INSTALL_PREFIX="$PWD/install"
cmake --build build
cmake --install build --prefix "$PWD/install"

# k4TauAnalysis
cd "$ROOT_DIR/k4TauAnalysis"
[ -f build/CMakeCache.txt ] || cmake -S . -B build -G Ninja \
  -DCMAKE_INSTALL_PREFIX="$PWD/install" \
  -DCMAKE_PREFIX_PATH="$ROOT_DIR/TauTool/install"
cmake --build build
cmake --install build
ctest --test-dir build --output-on-failure
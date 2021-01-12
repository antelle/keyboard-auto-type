#!/bin/bash
set -euo pipefail

cppcheck --enable=all --inline-suppr keyboard-auto-type

rm -rf build

cmake -B build -D RUN_CLANG_TIDY=1 .
cmake --build build

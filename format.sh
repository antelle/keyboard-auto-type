#!/bin/bash
set -euo pipefail

find example keyboard-auto-type -name '*.cpp' -o -name '*.h' | \
    xargs clang-format -i --verbose

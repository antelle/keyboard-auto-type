#!/bin/bash
set -euxo pipefail

cmake \
    -G Xcode \
    -B xcode \
    -D CMAKE_C_COMPILER="$(xcrun -find c++)" \
    -D CMAKE_CXX_COMPILER="$(xcrun -find cc)" \
    .

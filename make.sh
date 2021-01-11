#!/bin/bash
set -euo pipefail

cmake -B build .
cmake --build build

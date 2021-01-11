#!/bin/bash
set -euo pipefail

cppcheck --enable=all --inline-suppr keyboard-auto-type

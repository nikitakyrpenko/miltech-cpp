#!/usr/bin/env bash
# Usage: run-tests.sh [label]
#
# Arguments:
#   label  (optional) test label to filter by (e.g. homework_06)
#          if omitted, runs all tests
#
# Exit codes:
#   0  all tests passed
#   1  one or more tests failed

cd "$(dirname "$0")/.." || exit 1

cmake --preset debug
cmake --build --preset debug

if [ -z "$1" ]; then
    ctest --test-dir build/debug --output-on-failure
else
    ctest --test-dir build/debug --output-on-failure -L "$1"
fi

rm -rf build/debug/Testing

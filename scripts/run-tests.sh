#!/usr/bin/env bash
cd "$(dirname "$0")/.." || exit 1

cmake --preset debug
cmake --build --preset debug

if [ -z "$1" ]; then
    ctest --test-dir build/debug --output-on-failure
    rm -rf build/debug/Testing
    exit 0
fi

ctest --test-dir build/debug/"$1" --output-on-failure
rm -rf build/debug/"$1"/Testing
exit 0
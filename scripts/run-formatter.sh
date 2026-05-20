#!/usr/bin/env bash
# Usage: run-formatter.sh [path]
#
# Arguments:
#   path  (optional) subdirectory to format (e.g. homework_06)
#         if omitted, formats all homework_* directories
#
# Exit codes:
#   0  formatting complete

cd "$(dirname "$0")/.." || exit 1

if [ -z "$1" ]; then
    find homework_* -type f -regex '.*\.\(cpp\|hpp\)' -exec clang-format --style=file:.devcontainer/.clang-format -i {} +
    find homework_* -type f -name 'CMakeLists.txt' -exec cmake-format -c .devcontainer/.cmake-format.json -i {} +
    exit 0
fi

find "$1" -type f -regex '.*\.\(cpp\|hpp\)' -exec clang-format --style=file:.devcontainer/.clang-format -i {} +
find "$1" -type f -name 'CMakeLists.txt' -not -path "*/build/*" -exec cmake-format -c .devcontainer/.cmake-format.json -i {} +

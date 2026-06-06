# Usage: run-tidy.sh [path]
#
# Arguments:
#   path  (optional) subdirectory to analyze (e.g. homework_06)
#         if omitted, analyzes all homework_* directories
#
# Exit codes:
#   0  no errors
#   1  clang-tidy errors found

cd "$(dirname "$0")/.." || exit 1

if [ -z "$1" ]; then
    output=$(run-clang-tidy -p build/debug homework_* 2>&1)
    errors=$(echo "$output" | grep "error:")
    if [ -n "$errors" ]; then
        echo "$errors"
        exit 1
    fi
    exit 0
fi

output=$(run-clang-tidy -p build/debug "$1" 2>&1)
errors=$(echo "$output" | grep "error:")

if [ -n "$errors" ]; then
    echo "$errors"
    exit 1
fi

exit 0
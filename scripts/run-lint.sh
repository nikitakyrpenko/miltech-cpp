cd "$(dirname "$0")/.." || exit 1

output=$(run-clang-tidy -p build/debug homework_06 2>&1)
errors=$(echo "$output" | grep "error:")

if [ -n "$errors" ]; then
    echo "$errors"
    exit 1
fi
exit 0
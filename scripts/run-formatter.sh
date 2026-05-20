cd "$(dirname "$0")/.." || exit 1

find homework_* -type f -regex '.*\.\(cpp\|hpp\)' -exec clang-format --style=file:.devcontainer/.clang-format -i {} +
find . -type f -name 'CMakeLists.txt' -not -path "*/build/*" -exec cmake-format -c .devcontainer/.cmake-format.json -i {} +
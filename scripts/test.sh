#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
output="${TMPDIR:-/tmp}/dslibris-visual-test"
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude test/visual_test.cpp -o "$output"
"$output"

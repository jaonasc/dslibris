#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
sh scripts/test.sh
sh test/host-build.sh
python3 test/run_stability.py
sh test/host-font-build.sh
python3 test/run_font_test.py
sh test/book-bounds-build.sh
g++ -std=c++11 -Wall -Wextra -Werror -fsanitize=address,undefined -Iinclude test/storage_test.cpp -o /tmp/dslibris-host/storage-test
cd /tmp/dslibris-host
./storage-test

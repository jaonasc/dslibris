#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
headers="${DSLIBRIS_HOST_HEADERS:-/usr/include}"
mkdir -p /tmp/dslibris-host
g++ -std=c++11 -g -O1 -ffunction-sections -fdata-sections -fsanitize=address,undefined -Itest/host -Iinclude -I"$headers" -I"$headers/freetype2" \
    test/book_bounds_test.cpp source/book.cpp source/page.cpp -Wl,--gc-sections -o /tmp/dslibris-host/book-bounds-test
/tmp/dslibris-host/book-bounds-test

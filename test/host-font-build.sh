#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
headers="${DSLIBRIS_HOST_HEADERS:-/usr/include}"
mkdir -p /tmp/dslibris-host
g++ -std=c++11 -g -O1 -ffunction-sections -fdata-sections -fsanitize=address,undefined -DHOST_REAL_TEXT -Itest/host -Iinclude -I"$headers" -I"$headers/freetype2" \
    test/font_test.cpp test/host/runtime.cpp source/text.cpp source/prefs.cpp source/book.cpp source/page.cpp \
    -Wl,--gc-sections -l:libfreetype.so.6 -o /tmp/dslibris-host/font-test

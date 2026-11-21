#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
headers="${DSLIBRIS_HOST_HEADERS:-/usr/include}"
mkdir -p /tmp/dslibris-host
gcc -g -O1 -ffunction-sections -fdata-sections -fsanitize=address,undefined -Iinclude -c source/unzip.c -o /tmp/dslibris-host/unzip.o
gcc -g -O1 -ffunction-sections -fdata-sections -fsanitize=address,undefined -Iinclude -c source/ioapi.c -o /tmp/dslibris-host/ioapi.o
g++ -std=c++11 -g -O1 -ffunction-sections -fdata-sections -fsanitize=address,undefined -Itest/host -Iinclude -I"$headers" -I"$headers/freetype2" \
    test/stability_test.cpp test/host/runtime.cpp source/book.cpp source/page.cpp source/epub.cpp source/parser.cpp source/parse_state.cpp source/prefs.cpp source/log.cpp /tmp/dslibris-host/unzip.o /tmp/dslibris-host/ioapi.o \
    -Wl,--gc-sections -lexpat -lz -o /tmp/dslibris-host/stability-test

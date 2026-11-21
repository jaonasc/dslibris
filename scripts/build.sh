#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
export DEVKITPRO="${DEVKITPRO:-/tmp/dslibris-sdk/opt/devkitpro}"
export DEVKITARM="$DEVKITPRO/devkitARM"
export PATH="$DEVKITARM/bin:$DEVKITPRO/tools/bin:$PATH"
exec make -j4 "$@"

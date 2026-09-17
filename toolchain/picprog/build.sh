#!/bin/bash
# build.sh - compiles the vendored copy of picprog 1.9.1.
#
# The copy in src/ carries a local patch: a PIC18F25K80 entry in the device
# table (hexfile.cc, device_id 0x6180, write_size 64). The resulting executable
# is placed in bin/picprog and bundled with the platform package.
#
# Usage: ./build.sh
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/src"
BIN="$HERE/bin"

mkdir -p "$BIN"
make -C "$SRC" CXX="${CXX:-g++}" picprog
install -m 755 "$SRC/picprog" "$BIN/picprog"
make -C "$SRC" clean

echo "OK: $BIN/picprog"

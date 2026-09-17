#!/bin/bash
# build.sh - kompiluje osadzoną (vendored) kopię picprog 1.9.1.
#
# Kopia w src/ zawiera lokalny patch: dodany wpis PIC18F25K80 w tablicy
# urządzeń (hexfile.cc, device_id 0x6180, write_size 64). Wynikowy plik
# wykonywalny trafia do bin/picprog i jest dołączany do paczki platformy.
#
# Użycie: ./build.sh
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$HERE/src"
BIN="$HERE/bin"

mkdir -p "$BIN"
make -C "$SRC" CXX="${CXX:-g++}" picprog
install -m 755 "$SRC/picprog" "$BIN/picprog"
make -C "$SRC" clean

echo "OK: $BIN/picprog"

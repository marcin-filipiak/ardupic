#!/bin/bash
# build.sh - builds a HEX image for PIC18F25K80 from a C file using SDCC + gputils.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"

SDCC_BIN="${SDCC_BIN:-$HERE/sdcc/bin}"
GPUTILS_BIN="${GPUTILS_BIN:-$HERE/gputils/bin}"
LKR_PATH="${LKR_PATH:-$HERE/lkr}"
SDCC_LIB="${SDCC_LIB:-$HERE/sdcc/share/sdcc/lib/pic16}"
DEV_LIB="${DEV_LIB:-$HERE/lib}"
SDCC_INC="${SDCC_INC:-$HERE/sdcc/share/sdcc/include/pic16}"
GPUTILS_INC_DIR="${GPUTILS_INC_DIR:-$HERE/header}"

SRC="${1:?usage: $0 file.c}"
BASE="${SRC%.c}"

"$SDCC_BIN/sdcc" -S -mpic16 -p18f25k80 -I"$SDCC_INC" -I"$HERE" --std-c99 --no-warn-non-free -o "$BASE.asm" "$SRC"
"$GPUTILS_BIN/gpasm" -c -I"$DEV_LIB" -o "$BASE.o" "$BASE.asm"

# sdcc 4.5.0 quirk: old eeprom_gptr* name -> eeprom8_gptr*
SHIM_O="$DEV_LIB/gptr_shim.o"
if [ ! -f "$SHIM_O" ]; then
  "$GPUTILS_BIN/gpasm" -p18f25k80 -c -I "$GPUTILS_INC_DIR" -o "$SHIM_O" "$HERE/lib/gptr_shim.S"
fi

GPUTILS_LKR_PATH="$LKR_PATH" GPUTILS_LIB_PATH="${DEV_LIB}:${SDCC_LIB}" \
  "$GPUTILS_BIN/gplink" -w -r -o "$BASE.hex" \
  -I"$SDCC_LIB" -I"$DEV_LIB" \
  "$BASE.o" crt0iz.o gptr_shim.o libc18f.lib libio18f25k80.lib libdev18f25k80.lib libsdcc.lib
echo "OK: $BASE.hex"

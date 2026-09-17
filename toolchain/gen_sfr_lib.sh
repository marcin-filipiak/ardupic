#!/bin/bash
# Regenerates libdev<device>.lib - the SFR symbol library for SDCC/gplink.
# Usage: gen_sfr_lib.sh [device]  (default 18f25k80)
#
# SDCC emits references to SFRs as external symbols (e.g. _LATB).
# gputils 1.4 does not allow defining an absolute global symbol via "EQU"
# in relocatable mode (Error[156]). Solution: one absolute UDATA section per
# register address (covered by PROTECTED in the .lkr), with global _Name labels
# and RES 1.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
GPUTILS_BIN="${GPUTILS_BIN:-/tmp/opencode/gputils-deb/usr/bin}"

DEV="${1:-18f25k80}"
INC="${INC:-$HERE/header/p$DEV.inc}"
OUT_DIR="${OUT_DIR:-$HERE/lib}"
mkdir -p "$OUT_DIR"

python3 - "$INC" "$DEV" > "$OUT_DIR/sfr.asm" <<'PYEOF'
import re, sys
inc, dev = open(sys.argv[1]).read(), sys.argv[2]
block = re.search(r';----- Register Files.*?(?=\n;----- .* Bits)', inc, re.S).group(0)
regs = {}
for m in re.finditer(r"^\s*([A-Z][A-Z0-9_]*)\s+EQU\s+H'0([0-9A-F]{2,3})'\s*$", block, re.M):
    a = int(m.group(2), 16)
    regs.setdefault(a, []).append(m.group(1))
lines = [" list p=%s" % dev, " radix hex"]
for a in sorted(regs):
    lines.append("S%03X UDATA 0x%03X" % (a, a))
    for n in sorted(regs[a]):
        lines.append(" GLOBAL _%s" % n)
        lines.append("_%s:" % n)
    lines.append(" RES 1")
lines.append(" END")
sys.stdout.write("\n".join(lines) + "\n")
PYEOF

"$GPUTILS_BIN/gpasm" -c -o "$OUT_DIR/sfr.o" "$OUT_DIR/sfr.asm"
"$GPUTILS_BIN/gplib" -q -c "$OUT_DIR/libdev$DEV.lib" "$OUT_DIR/sfr.o"
echo "OK: $(ls -la "$OUT_DIR/libdev$DEV.lib")"

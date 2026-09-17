#!/bin/bash
# Regeneruje libdev18f25k80.lib — bibliotekę symboli SFR dla SDCC/gplink.
#
# SDCC generuje odwołania do SFR jako symboli zewnętrznych (np. _LATB).
# gputils 1.4 nie pozwala zdefiniować absolutnego symbolu globalnego przez
# "EQU" w trybie relokowalnym (Error[156]). Rozwiązanie: jedna absolutna
# sekcja UDATA na adres rejestru (0xE41..0xFFF pokryte przez PROTECTED
# DATABANK w .lkr), z globalnymi labelami _Nazwa i RES 1.
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
GPUTILS_BIN="${GPUTILS_BIN:-/tmp/opencode/gputils-deb/usr/bin}"
INC="${INC:-$HERE/header/p18f25k80.inc}"
OUT_DIR="${OUT_DIR:-$HERE/lib}"
mkdir -p "$OUT_DIR"

python3 - "$INC" > "$OUT_DIR/sfr.asm" <<'PYEOF'
import re, sys
inc = open(sys.argv[1]).read()
block = re.search(r';----- Register Files.*?(?=\n;----- .* Bits)', inc, re.S).group(0)
regs = {}
for m in re.finditer(r"^\s*([A-Z][A-Z0-9_]*)\s+EQU\s+H'0([0-9A-F]{2,3})'\s*$", block, re.M):
    a = int(m.group(2), 16)
    regs.setdefault(a, []).append(m.group(1))
lines = [" list p=18f25k80", " radix hex"]
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
"$GPUTILS_BIN/gplib" -q -c "$OUT_DIR/libdev18f25k80.lib" "$OUT_DIR/sfr.o"
echo "OK: $(ls -la "$OUT_DIR/libdev18f25k80.lib")"
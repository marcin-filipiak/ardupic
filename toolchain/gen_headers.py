#!/usr/bin/env python3
"""Generuje <device>.h (deklaracje SFR dla SDCC) z pliku .inc gputils.

Uzycie:
    gen_headers.py <inc> <out.h> <DEVICE>

Parsuje blok ";----- Register Files" z pliku .inc Mikrochipa i wypisuje
deklaracje SFR w stylu projektu:
    extern __at (0x0F62) __sfr NAME;
Adresy narastajaco, aliasy tego samego adresu wypisane po kolei.
"""
import re, sys

inc_path, out_path, device = sys.argv[1:4]

inc = open(inc_path).read()
block = re.search(r";----- Register Files.*?(?=\n;----- .* Bits)", inc, re.S).group(0)
by_addr = {}
for m in re.finditer(r"^\s*([A-Z][A-Z0-9_]*)\s+EQU\s+H'0([0-9A-F]{2,3})'\s*$", block, re.M):
    a = int(m.group(2), 16)
    by_addr.setdefault(a, []).append(m.group(1))

lines = [
    "/* %s.h - deklaracje SFR dla SDCC (generowane, adresy z gputils %s.inc) */" % (device, device),
    "#ifndef _%s_H" % device.upper(),
    "#define _%s_H" % device.upper(),
    "",
]
for a in sorted(by_addr):
    for n in sorted(by_addr[a]):
        lines.append("extern __at (0x%04X) __sfr %s;" % (a, n))
lines += ["", "#endif"]

open(out_path, "w").write("\n".join(lines) + "\n")
print("OK: %d adresow SFR -> %s" % (len(by_addr), out_path))